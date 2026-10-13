# Dyninst Testsuite — Test Purpose & Failing-Condition Report

This report covers every mutator test under `src/` (components: DyninstAPI, InstructionAPI,
ProcControlAPI, SymtabAPI, StackwalkerAPI). For each test (or family) it states what the
test verifies and what concretely makes it report **FAILED**. A final section lists
oddities and known quirks found while reading the tests.

## How the suite works (shared across everything)

Most tests are a **mutator/mutatee pair**: the mutator (`testN_M.C`) is a `TestMutator`
subclass whose `executeTest()` uses a Dyninst API against a target program; the mutatee
(`testN_M_mutatee.c`) is the target that runs the instrumented code and self-checks the
results, calling `test_passes(testname)` or logging `**Failed**`. A test returns
`PASSED`, `FAILED`, or `SKIPPED` (unsupported platform/run-mode/compiler).

Two failure surfaces exist almost universally; per-test entries below only add what is
specific to that test:

- **Mutator-side** (applies to nearly every DyninstAPI test):
  - `appImage->findFunction(...)` returns NULL, an empty vector, or the wrong number of
    matches (many tests require *exactly one* match and fail on duplicates too);
  - `findPoint(BPatch_entry/exit/subroutine/...)` returns NULL or an empty vector;
  - `findVariable`/`findType` returns NULL;
  - `insertSnippet`/`replaceFunction`/`replaceFunctionCalls`/`loadLibrary`/`malloc`
    (inferior) returns NULL/false/negative;
  - `processCreate`/`processAttach` returns NULL;
  - an expected callback never fires before a timeout, or the mutatee terminates
    prematurely while the mutator is still waiting.
- **Mutatee-side**: after the instrumented function runs, the mutatee compares globals
  against expected magic values written by the inserted snippets; any mismatch fails.
  A global left at its initialized value means the snippet never executed; a wrong
  non-initial value means the snippet executed incorrectly.

---

# 1. DyninstAPI tests (`src/dyninst/`)

## 1.1 test1_* — basic instrumentation (BPatch snippets)

- **test1_1** — *Insert a call to a zero-argument function at function entry.*
  Fails when: mutator can't find `test1_1_func1_1`, its entry point, or `test1_1_call1_1`,
  or `insertSnippet` returns NULL. Mutatee: fails if `globalVariable1_1 != 11` (the
  inserted call sets it to 11; 11 not seen ⇒ snippet never ran).
- **test1_2** — *Insert a call with four constant arguments (int, int, string, pointer).*
  Fails when: function/entry/`test1_2_call2_1` lookup or either `insertSnippet` fails.
  Mutatee: fails unless `arg1==1 && arg2==2 && arg3=="testString2_1" && arg4==` the
  expected pointer value.
- **test1_3** — *Pass mutatee variables as call arguments: one global, one
  mutator-side inferior-`malloc`'d int.* The mutator allocates the int and writes 32 into
  it. Fails when: func/entry/`test1_3_call3_1`/subroutine points/`globalVariable3_1`
  lookup fails, inferior `malloc` fails, or either insert fails. Mutatee: fails unless
  the callee receives `arg1==31 && arg2==32`.
- **test1_4** — *`BPatch_sequence` ordering:* assign `globalVariable4_1=42` then `=43` in
  a single sequence at entry. Fails when: variable not found or insert fails. Mutatee:
  distinguishes three outcomes — value 41 = snippet never ran (FAIL), 42 = statements ran
  in the wrong order (FAIL), 43 = PASS.
- **test1_5** — *`BPatch_ifExpr` without else,* 17 predicates covering constant and
  relational comparisons including INT/LLONG/ULLONG limit values. Fails when: any of
  `globalVariable5_1..5_17` or the points are missing, or an insert fails. Mutatee: each
  `if` is constructed so a correct outcome leaves its global 0 — fails if **any** of the
  17 globals is nonzero.
- **test1_6** — *Arithmetic operators* (+, −, ×, ÷, sequence, xor) with immediate
  constants, const variables, and 64-bit/unsigned operands. Fails when: any
  `globalVariable*`/`constVar*` missing or insert fails. Mutatee: fails on any result
  mismatch, e.g. `globalVariable1==62`, `2==63`, `3==22`, `5==30`, `8==(5^9)`,
  `5b==6442450941` (64-bit), `10b==ULLONG_MAX`.
- **test1_7** — *Relational operators* (lt, gt, eq, ne, le, ge, and, or) guarding
  assignments, with both constants and const variables. Fails when: `test1_7_func2`/entry
  or any operand variable missing, or insert fails. Mutatee: fails unless every
  true-case global reads 72 (constant operands) / 74 (variable operands) and every
  false-case global reads 71 / 73.
- **test1_8** — *Register preservation across a deeply nested arithmetic expression*
  (sum of constants 81..88). Fails when: `test1_8_func1`/entry/`globalVariable1` missing
  or insert fails. Mutatee: fails if `globalVariable1 != 676` **or** the instrumented
  function's own parameters p1..p10 no longer read 1..10 (i.e. the snippet clobbered
  registers holding live values).
- **test1_9** — *Register preservation across a function-call snippet with 5 arguments,*
  using five inferior-`malloc`'d ints the mutator sets to 91..95. Fails when: lookups or
  any of the mallocs/inserts fail. Mutatee: fails if the callee's received sum
  `x != 91+92+93+94+95`, any individual p1..p5 is wrong, or the surrounding function's
  params p1..p10 were clobbered.
- **test1_10** — *Snippet insertion ordering* (`BPatch_firstSnippet`/`lastSnippet`):
  insert call2 normally, then call1 as *first*, then call3 as *last*. Fails when: any
  of `test1_10_call1/2/3` or the point is missing, or any insert fails. Mutatee: the
  three calls record their execution order; fails unless
  `globalVariable10_1==1 && 10_2==1 && 10_3==1 && 10_4==3` (each call ran exactly once,
  in first/middle/last order).
- **test1_11** — *Snippets at all four point types:* entry, call-before, call-after,
  exit. Fails when: entry/subroutine/exit points or `test1_11_call1..call4` missing, or
  any insert fails. Mutatee: `globalVariable11_1` acts as a phase counter (0→1→2→3);
  fails unless each of `globalVariable11_2/3/4/5` proves its snippet fired during the
  correct phase.
- **test1_12** — *Insert & delete snippet, inferior malloc/free, heap-exhaustion.*
  Custom execution path. Fails when: func/entry missing; `malloc(100)` (inferior) fails;
  the deliberate heap-exhaustion stress `malloc` does not produce expected error 66;
  insert fails; `deleteSnippet(handle)` returns an error; **or** `deleteSnippet(NULL)`
  wrongly reports success (negative test).
- **test1_13** — *`BPatch_paramExpr` (5 params), `BPatch_nullExpr`, `BPatch_retExpr`.*
  Fails when: func/entry, `test1_13_call1`, `test1_13_func2` exit, or `test1_13_call2`
  missing, or inserts fail (especially the retExpr insertion at exit). Mutatee: fails if
  the parameters don't arrive as 131..135, `globalVariable13_1 != 63`, or func2's return
  value was clobbered (`!= RET13_1`).
- **test1_14** — *`replaceFunctionCalls`:* (a) replace a call's target, (b) remove a call
  (NULL target), (c) replace with a function from dynamically loaded libtestA. Fails
  when: any `replaceFunctionCalls` returns < 0 or `loadLibrary(libtestA)` fails. Mutatee:
  fails unless `globalVariable14_1==1` (replaced call ran), `14_2==0` (removed call never
  ran), `14_3==2` (library replacement ran).
- **test1_15** — *`BPatch_whileExpr` loops:* increment to 10, double to 1024, decrement
  to 0, halve to 1, and a never-taken loop. Fails when: functions/points or
  `globalVariable15_1..15_5` missing, or inserts fail. Mutatee: fails unless the five
  globals read exactly `10, 1024, 0, 1, 0`.
- **test1_16** — *`BPatch_ifExpr` with else,* including generated long expression bodies
  ("genLongExpr") to force branch-displacement handling, across func2/func3/func4. Fails
  when: any func/entry or `globalVariable16_1..16_10` missing, or `insertSnippetAt`
  fails. Mutatee: fails unless each taken clause set its global to 1 **and** the
  corresponding not-taken clause left its global 0 (e.g. `16_1==1 && 16_2==0`,
  `16_3==0 && 16_4==1`).
- **test1_17** — *Exit-point instrumentation must not clobber return values:* insert
  calls at func1/func2 exits. Fails when: funcs/exit points/`call17_1/2` missing or
  inserts fail. Mutatee: fails unless `ret17_1==RET17_1`,
  `globalVariable17_1==RET17_2`, and `globalVariable17_2==RAN17_1`.
- **test1_18** — *`readValue`/`writeValue` on a mutatee global.* Fails when:
  `test1_18_func1`/subroutine point or `globalVariable1` missing; `readValue` returns a
  value ≠ 42 (≠ 0 for the Fortran variant); or `writeValue(17)` returns false.
- **test1_19** — *`oneTimeCode` (synchronous) and `oneTimeCodeAsync` with completion
  callback.* Custom path. Fails when: `test1_19_call1`/`call2` missing;
  `waitUntilStopped` fails; the async completion callback never fires (`callbackFlag`
  stays 0); or the process terminates while the mutator is still waiting.
- **test1_20** — *`BPatch_arbitrary` points inside basic blocks via the CFG,* inserted
  through an insertion set. Fails when: `test1_20_call1`/`func2` missing; the function
  has no CFG or zero basic blocks; a block pointer is NULL; `findPoint(nullFilter)`
  returns NULL; an insert at an arbitrary point returns NULL; or **no arbitrary point
  was found at all** (`found_one` stays false).
- **test1_21** — *Module-scoped `findFunction` + regex module search* across loaded
  libtestA/libtestB. Fails when: `loadLibrary` fails; `getObjects` is empty; module A or
  B can't be located; `call21_1` not found in each module; the two copies of `call21_1`
  share a base address (must be distinct functions); or regex search `^cb` doesn't match
  exactly 2 functions / `^cbll21` exactly 1. SKIPPED off Linux/Windows/FreeBSD.
- **test1_22** — *`replaceFunction` in all four directions:* a.out→a.out, a.out→shlib,
  shlib→shlib, shlib→a.out (via libtestA/libtestB). Fails when: `loadLibrary` fails,
  modules or any `call22_*` missing, or any `replaceFunction` returns false. Mutatee:
  fails unless each replaced pair updates its result global through the *replacement*
  (e.g. `result == 10+MAGIC22_2`, `== 20+MAGIC22_4`, etc. — the original function's
  value appearing means replacement didn't take).
- **test1_23** — *Local variables and shadowed variables* (local shadowing a global).
  Fails when: `test1_23_call1`/subroutine points missing; `localVariable23_1`,
  `test1_23_shadowVariable1` (the local), `test1_23_shadowVariable2`/`globalVariable1`
  (the globals) not resolvable at the right scopes; or inserts fail. Mutatee: fails
  unless local==2300001, local shadow==2300012, global shadow==2300023, and
  `globalVariable1` received the copy of the local's value.
- **test1_24** — *Array references:* constant index, variable index, multi-dimensional
  arrays. Fails when: `test1_24_call1`/points, `globalVariable1..9`, or
  `localVariable24_1` missing; or any of the 10 reference-assignment inserts fails.
  Mutatee: fails unless the array elements/result globals hold the expected 2400001..
  series. SKIPPED off x86/x86_64 Linux/Windows/FreeBSD and for Fortran mutatees.
- **test1_25** — *Unary operators* (address-of, dereference, negate) with type-checking
  disabled. Fails when: `test1_25_call1`/entry or `globalVariable1..7` missing, or
  `findType("void *")` fails on platforms where it's exercised. Mutatee: fails unless
  `gv2 == &gv1`, `gv3 == *gv2 (== gv1)`, `gv5 == -gv4`, `gv7 == -gv6`.
- **test1_26** — *Struct field access via `getComponents`:* plain fields, an array
  field, and a nested-struct field, on both a global and a local struct. Fails when:
  `test1_26_call1`/points, `globalVariable1..13`, or `localVariable26_1` missing;
  `getComponents` returns NULL; the struct has fewer than 4 fields; field names are not
  `field1..field4`; a subfield is NULL; or inserts fail. SKIPPED for Fortran.
- **test1_27** — *Type compatibility (`isCompatible`).* Fails when: `test1_27_type1..4`
  not found; a pair expected to be compatible reports incompatible; a pair expected to be
  incompatible reports compatible (each incompatible probe must also raise expected
  error 112); or `test1_27_mutatee`/entry/`globalVariable5..8`/`globalVariable1`
  missing. On success writes `globalVariable1=1`. SKIPPED for Fortran.
- **test1_28** — *User-defined types* (`createStruct/Union/Typedef/Array/Scalar/Enum`)
  and applying them with `setType`; union read/write; enum compatibility. Fails when:
  `findType("int")` fails; `globalVariable1`/`globalVariable8`/`test1_28_call1` missing;
  created-struct field names are wrong; after writing union fields the read-back values
  are not 1 and 2; the created scalar's size ≠ 8; enum creation fails; two identically
  defined enums report incompatible; or two different enums report compatible.
  SKIPPED for Fortran.
- **test1_29** — *Source-object hierarchy* (`getSourceObj`/`getSrcType`:
  program→module→function). Fails when: walking the hierarchy encounters an object of
  the wrong `getSrcType` at any level; `test1_29_func1`/entry/`globalVariable1` missing;
  or `writeValue(1)` fails.
- **test1_30** — *Line/address information:* `getAddressRanges` on both image and
  module, `getSourceLines`, `BPatch_function::getAddressRange`. Fails when:
  `test1_30_mutatee`/entry/`test1_30_call1` or `globalVariable3..9` missing; or
  `getAddressRanges` returns false at either scope. Mutatee: fails unless the
  line/address globals the mutator filled in are mutually consistent and nonzero
  (e.g. `globalVariable6 == globalVariable1`). SKIPPED for Fortran.
- **test1_31** — *Non-recursive base-tramp guard* (`setTrampRecursive(false)`):
  instrument func2 entry → call func3; func3 entry/exit → call func4. Fails when:
  `test1_31_func2/3/4` missing. Mutatee: fails unless `globalVariable31_3 == 1`
  (instrumentation ran) **and** `globalVariable31_4 == 0` (the guard suppressed
  instrumentation-triggered recursion — nonzero means the guard leaked).
- **test1_32** — *Recursive base tramp* (`setTrampRecursive(true)`), same layout as
  test1_31. Fails when: `test1_32_func2/3/4` missing. Mutatee: fails unless
  `globalVariable32_3 == 1` and `globalVariable32_4 == 3` (recursion now *expected*;
  fewer than 3 means recursive instrumentation was wrongly suppressed).
- **test1_33** — *CFG correctness.* Structural assertions on func2/func3: exactly one
  entry and one exit block; in/out-degree ≤ 2 with exactly two degree-two blocks; no
  back-edges/loops in func2; the switch in func3 discovered with its in/out blocks; the
  entry block dominates all blocks; the exit block postdominates all blocks. Fails when:
  functions/CFG missing, any structural check fails, or `globalVariable1` missing /
  write fails. SKIPPED for Fortran.
- **test1_34** — *Loop nesting information* (`getLoops`, `getContainedLoops`,
  `getOuterLoops`). Fails when: `test1_34_func2`/CFG missing; total loop count ≠ 4; no
  loop contains exactly 3 loops; or the contained/outer-loop counts don't match the
  expected 1/3/2/0 nesting pattern.
- **test1_36** — *Callsite parameter reading:* `BPatch_paramExpr` at a direct call site
  with 10 arguments, plus enumeration of indirect callees which must be exactly
  `malloc, strncpy, toupper, strncpy, free`. Fails when: `test1_36_func1`/subroutine
  points missing; the `test1_36_call1` callsite not found; any `globalVariable1..10`
  missing; the indirect-call point count ≠ 1; callee count ≠ 5; a `getCalledFunction`
  returns NULL; or a callee name mismatches. Mutatee: fails unless `result == 55`
  (sum 1..10) and `globalVariable1..10 == 1..10`.
- **test1_37** — *Loop instrumentation* via `findLoopInstPoints` on loop entry/exit
  edges (increment a global / call an incrementing function in loop bodies). Fails
  when: loop entry/exit point vectors are empty; any insert at a loop point returns
  NULL; or functions/variable missing. Mutatee: fails unless
  `globalVariable37_1/2/3 == ANSWER37_1/2/3` (each encodes the exact expected iteration
  count × instrumentation hits). SKIPPED for Fortran.
- **test1_38** — *Loop/callee tree* (`getLoopTree`, `numCallees`, `getCalleeName`,
  `children`). Fails when: `test1_38_call1`/CFG missing; the tree has no children; the
  child/callee count at any nesting level is wrong; or the callee names at each node
  aren't the expected `funCall38_1..funCall38_7`; or `globalVariable2` missing.
  SKIPPED for Fortran.
- **test1_39** — *Regex function search.* Fails when: `^test1_39_` doesn't match exactly
  2 functions; `^test1_39_func1` doesn't match exactly 1; the libc pattern `^sp`
  matches nothing; or `test1_39_passed` missing (on success the mutator writes 1 into
  it). SKIPPED off Linux/FreeBSD (e.g. Windows).
- **test1_40** — *Monitoring dynamic (indirect) call sites* (`monitorCalls`): the
  dispatcher `call5` must contain exactly one dynamic call point; a monitor function is
  registered for it. Fails when: any of `test1_40_call1/2/3/5` or the monitor function
  missing; `findPoint(subroutine)` NULL; dynamic-call-point count ≠ 1; the `setVar40`
  write fails; or `monitorCalls` returns false. Mutatee: fails unless the addresses the
  monitor recorded equal the real callee addresses
  (`addr_of_call1 == gv40_call40_1_addr`, etc., and the dispatcher callsite address
  matches for all three monitored calls). SKIPPED for XLC-compiled and Fortran mutatees.
- **test1_41** — *Line-information stability across runs:* launches the same mutatee
  twice (`-run test1_41`) and compares `Module::getStatements` counts. Fails when:
  `processCreate` fails; no image; the module (`test1_41_mutatee.c` /
  `solo_mutatee_boilerplate.c`) not found; `getStatements` returns false; or the
  statement counts differ between the two runs. SKIPPED for Fortran.

*(There is no test1_35; the numbering skips it.)*

## 1.2 test2_* — error handling & API misuse

Shared pattern: bracket an API call with `clearError()`/`setExpectError()`, call an API
that is **expected to fail**, and require both a failed return *and* a fired error
callback — the canonical check is `FAILED if (ret != NULL || !gotError)`. Positive
subtests (2_6..2_14) exercise a real capability and on success write 1 into a mutatee
variable `test2_N_passed` via `findVariable(...)->writeValue()` (missing variable ⇒
FAILED). Several tests are create-only or attach-only and return SKIPPED otherwise.

- **test2_1** — *`processCreate` on a nonexistent executable* (`./noSuchFile`). Fails
  if a process handle is returned **or** the error callback didn't fire. Create-mode
  only (SKIPPED under attach). No mutatee runs.
- **test2_2** — *`processCreate` on a non-executable file* (`/dev/null`; `nul:` on
  Windows). Same double condition: handle returned or no error callback ⇒ FAILED.
- **test2_3** — *`processAttach` to an invalid pid* (65539). Attach-mode only. Fails if
  the attach yields a handle or no error callback fires.
- **test2_4** — *`processAttach` to a protected pid* (pid 1). Attach-mode only; SKIPPED
  when running as root (root *can* attach, so the negative test is meaningless). Fails
  if attach succeeds or no error.
- **test2_5** — *`findFunction` on a nonexistent name* (`"NoSuchFunction"`), expecting
  specific error **#100**. Fails unless the lookup returns NULL/empty **and**
  `getError()` shows the error callback fired.
- **test2_6** — *Detect a mutatee-side `dlopen`:* the mutatee loads a shared library;
  after continuing to the stop point the mutator scans `getModules`. Fails if no module
  name matches `TEST_DYNAMIC_LIB`.
- **test2_7** — *Mutator-side `loadLibrary(TEST_DYNAMIC_LIB2)`,* verified via
  `getObjects`. Fails if `loadLibrary` reports an error, the loaded object doesn't
  appear, or `test2_7_passed` can't be found/written.
- **test2_8** — *`BPatch_breakPointExpr`:* insert a breakpoint snippet at the entry of
  `test2_8_mutatee` and wait for the process to stop. Fails if the function/point isn't
  found, the insert returns NULL, `waitUntilStopped` fails, or `test2_8_passed` is
  missing.
- **test2_9** — *`dumpCore("mycore", true)` without terminating the process.* Fails if
  `getError()` reports an error **or** the file `mycore` was not actually created on
  disk.
- **test2_10** — *`dumpImage("myimage")`.* Compile-time-guarded to i386/x86_64 Linux
  (effectively SKIPPED elsewhere). Fails if the process terminated early, an error was
  reported, the image file is absent, or `test2_10_passed` is missing.
- **test2_12** — *`BPatch_point` query functions* (`findPoint(BPatch_entry)` on
  `test2_12_func1`, then `usesTrap_NP()`). Mostly a smoke test: fails only if the
  function/points or `test2_12_passed` can't be found. The `usesTrap_NP` result itself
  is not asserted.
- **test2_13** — *`loadLibrary` failure reporting:* `loadLibrary("noSuchLibrary.Ever")`
  with a custom error callback capturing expected error **#124**. Fails if the load
  *succeeds*, no error string was captured by the callback, the mutatee terminated, or
  `test2_13_passed` is missing.
- **test2_14** — *Deleting a process object:* `killMutatee` then `delete appProc`; the
  deleted pointer must no longer appear in `getProcesses()`. Fails if the stale pointer
  is still listed.

*(There is no test2_11; the numbering skips it.)*

## 1.3 test3_* — multiple-process management

Shared pattern: build `child_argv = {pathname, [-verbose], -run, test3_N, NULL}`, spawn
`Mutatees` (= 3) copies via `processCreate` (or OS fork + `processAttach` for 3_6), drive
them via `continueExecution`/`waitForStatusChange`, and check termination bookkeeping:
`isTerminated()`, `terminationStatus()` (`ExitedNormally` vs `ExitedViaSignal`),
`getExitCode()`/`getExitSignal()`. Any `processCreate` returning NULL fails immediately
(with `MopUpMutatees` cleaning up the survivors).

- **test3_1** — *Simultaneous termination:* create 3, run ~5 s, `terminateExecution` on
  each. Fails unless every process ends up `isTerminated()` with
  `terminationStatus() == ExitedViaSignal` (`numTerminated == 3` required).
- **test3_2** — *Simultaneous normal exit:* create 3, let them run to completion,
  monitor with `waitForStatusChange`. Fails if any `processCreate` is NULL or fewer
  than 3 processes reach `isTerminated` with `ExitedNormally`/expected exit code.
- **test3_3** — *Different instrumentation per process:* in process *n*, insert
  `test3_3_ret = n` and a call `test3_3_call1(2, n)`; each mutatee writes its result to
  `test3.out.<pid>`. Fails when: create/find-function/find-variable/find-callsite
  fails; an insert returns NULL; or `readResult(pid) != n` for any process. Mutatee
  detail: `test3_3_ret` is initialized to `0xdeadbeef` — that value surviving means the
  instrumentation never executed.
- **test3_4** — *Sequential create → wait-for-exit,* 3 iterations. Fails **only** if a
  `processCreate` returns NULL; the loop otherwise always passes (see Oddities).
- **test3_5** — *Sequential create with mutatee `abort()`:* like 3_4 but the mutatee
  dies via `abort()`. Fails only if a `processCreate` returns NULL (see Oddities).
- **test3_6** — *Attach to externally forked processes:* processes are created by OS
  `fork`/`execv` (not `processCreate`), then `processAttach`. Fails when: a fork fails;
  any attach returns NULL; a process isn't terminated / has the wrong status; or the
  final liveness probe fails — `kill(pid, SIGKILL)` must return −1 (ESRCH: already
  dead) for every child, requiring `numTerminated == 3`. SKIPPED on Windows.
- **test3_7** — *Asynchronous one-time codes at scale:* fire 400 `oneTimeCodeAsync`
  calls (each invoking `test3_7_call1`) spread across 3 mutatees and count completion
  callbacks. Fails when: create/find fails; the done flag is never set because the
  callback count never reaches `TEST7_NUM_ONETIMECODE` (400); or a process doesn't
  terminate with the expected signal.

## 1.4 test4_* — fork/exec handling

Shared pattern: register `registerPreForkCallback`/`registerPostForkCallback`/
`registerExecCallback`/`registerExitCallback`, `processCreate` one mutatee, run with
`contAndWaitForAllProcs`. A static `passedTest` flag is set **only inside the exit
callback**, after `verifyChildMemory(proc, "<global>", <expected>)` (findVariable +
readValue + compare) and an exit-code-equals-`pid & 0xff` check. Universal failing
conditions: `processCreate` NULL; `!passedTest` at the end (the awaited callback chain
never completed); a process exited via signal; exit code ≠ pid; `verifyChildMemory`
mismatch; or a callback that should not fire firing (logged as failure). Callbacks are
unregistered at the end. 4_2–4_4 SKIPPED on Windows (`i386_unknown_nt4_0`).

- **test4_1** — *Exit callback, no fork/exec expected.* All four callbacks registered;
  only exit may fire. Fails if `test4_1_global1 != 1000001` at exit, exit code ≠
  `pid & 0xff`, exit via signal — or if the **exec** callback fires at all (explicitly
  logged as a failure, since no exec happens).
- **test4_2** — *Fork callback:* in post-fork, instrument parent (insert call to
  `test4_2_func3` at exit of func2) and child (`test4_2_func4`) **differently**. The
  callbacks call `exit(1)` outright if their `findFunction` fails. Fails unless **both**
  processes exit cleanly (`exited == 2`, code == pid, no signal) with parent global
  `2000002` and child global `2000003` (same-value results would mean the differential
  instrumentation leaked between processes).
- **test4_3** — *Exec callback:* the mutatee `exec`s; inside `execFunc` the mutator
  instruments the **post-exec image** (call to `test4_3_func2` at exit of
  `test4_3_func1`). `execFunc` calls `exit(1)` if the post-exec functions can't be
  found (image parsing after exec failed). Fails at exit unless
  `globalVariable3_1 == 3000002` in the exec'd process and no signal exit.
- **test4_4** — *Fork + exec combined:* post-fork instruments the parent; `execFunc`
  instruments the exec'd child (`test4_4_func4`). Fails unless both processes exit
  (`exited == 2`), no signal, exit code == `pid & 0xff`, parent
  `test4_4_global1 == 4000002`, exec'd child == `4000003`; an exit event from an
  unknown thread is also a failure.

## 1.5 test5_* — C++ feature instrumentation

Shared pattern: `findFunction("class::func_cpp")`, get subroutine/entry/exit points,
inspect C++ metadata (types, params, called-function names, struct components,
static-variable addresses), then `insertSnippet` a call to a mutatee "pass" function.
Mutatee side keeps `static int passed = 0`; the instrumented pass function sets
`passed = 1`, and the mutatee reports PASS only if `passed == 1` — so *any* silently
broken insertion shows up as a mutatee-side failure. Mutator fails on any NULL lookup,
`isCompatible` failure, wrong param/component count, or missing symbol.
(`cpp_test.C` is mutatee-side support recording per-subtest flags; not a mutator.)

- **test5_1** — *Argument passing:* at the `arg_test::call_cpp` subroutine point find
  locals `reference, arg1, arg2, arg3, m`; check `type1_1 ~ type1_3` and
  `type1_2 ~ type1_0` compatibility; assign `arg3 = 1`; insert
  `arg_test::arg_pass(&test1, 1)` at the `func_cpp` call site. Fails if any of the 5
  locals is missing, either compatibility check fails, or `test1` is missing.
- **test5_2** — *Overloaded functions:* the 3 call sites inside
  `overload_func_test::func_cpp` must all resolve to `overload_func_test::call_cpp`,
  with per-site parameter counts 1, 1, 2 (an extra leading `this` parameter is
  tolerated). Fails on fewer than 3 call points, no exit point, wrong resolved name, or
  wrong parameter count at any site.
- **test5_3** — *Overloaded operator:* among the subroutine points of `func_cpp`, find
  the call to `overload_op_test::operator++` and insert
  `::call_cpp(&test5_3_test3, retExpr)` at the operator's **exit**. Fails if the
  operator call can't be located among the call sites.
- **test5_4** — *Static member variables:* find two `static_test::call_cpp` call sites,
  read local `count` at each exit, and require both `getBaseAddr()` values to be
  **identical** (one shared static, not two instances). Fails on missing points/vars,
  fewer than 2 collected variables, or differing base addresses.
- **test5_5** — *Namespaces:* at `namespace_test::func_cpp` exit resolve
  `local_fn_var`, `local_file_var_5_5`, `CPP_DEFLT_ARG`; in `main` find the
  `test5_5_test5` struct, locate its `class_variable` field via components, insert
  `namespace_test::pass(&test5_5_test5)`. Fails if any of the three scoped variables,
  the struct, its fields, the constructor, or `::pass` is missing.
- **test5_6** — *Exception handling:* find local `testno` inside the try block; find
  the call to `sample_exception::response` in the **catch block** and insert
  `test5_6_passed(6)` at its exit. Fails if `testno` or the catch-block call is never
  found. Mutatee detail: `test5_6_passed(arg)` sets `passed = 1` only when
  `arg == 6` — a snippet that runs with a corrupted argument still fails.
- **test5_7** — *Templates:* find calls to both `sample_template<int>::content` and
  `sample_template<char>::content`; read local `ret` in each; the two `ret` types must
  be `isCompatible` with `int` and `char` respectively. Fails if either specialization
  is missing (`flag != 2`), a called function is NULL, a `ret` is missing, or a type
  compatibility check fails.
- **test5_8** — *Declarations/scoped resolution:* find `decl_test::func_cpp` exit,
  `main`, `decl_test::call_cpp`, and variable `test5_8_test8`; resolve `CPP_DEFLT_ARG`
  and `test5_8_test8` at different scopes (three scope-resolution expressions
  `expr8_2/3/4` must all be non-NULL); find struct field `CPP_TEST_UTIL_VAR`; insert
  `call_cpp(&test5_8_test8, 8)`. Fails on any missing lookup, NULL expression, empty
  struct, or missing field.
- **test5_9** — *Inheritance:* find `derivation_test::func_cpp` exit and the
  `test5_9_test9` struct in `main`; the struct's components must include the
  **inherited** member `call_cpp` (or `cpp_test_util::call_cpp`). Fails if the struct
  is empty or the inherited member isn't visible in the derived type's components.

## 1.6 test_fork_* — fork + instrumentation interaction

Shared pattern: create the process, register `registerPostForkCallback` /
`registerExitCallback`, insert/delete/allocate snippets in parent and/or child around a
`fork()`, then at each process's exit callback `verifyProcMemory(name, expected)`
(findVariable + readValue + compare) decides pass/fail. Universal failing conditions:
any lookup/insert/delete failure (routed through `doError`, which clears `passedTest`);
the post-fork handler never running (NULL child thread); or a final value mismatch in
either process. All SKIPPED on Windows. Helpers live in `test_lib_test7.C`
(SysV message-queue setup, `verifyProcMemory`, `doError`) and `test_lib_test9.C`
(`sleep_ms`); neither helper is a test itself.

- **test_fork_5** — *Delete snippet in parent:* insert "assign 321" pre-fork; the parent
  deletes its handle post-fork. Fails unless parent reads **123** (snippet removed
  before it ran) and child reads **321** (the inherited copy of the snippet must still
  execute in the child).
- **test_fork_6** — *Delete snippet in child:* insert "assign 951" pre-fork; the child
  deletes its inherited snippets post-fork. Fails if the child reports **no** inherited
  snippets (`getCurrentSnippets` empty — inheritance itself broken), a `deleteSnippet`
  returns false, or values mismatch: parent **951**, child **159**.
- **test_fork_7** — *Delete snippet in both:* insert "assign 642" pre-fork; both
  processes delete post-fork. Fails if either delete fails, the child has no snippets
  to delete, or either process reads anything but the uninstrumented **246**.
- **test_fork_8** — *Insert in child only:* no pre-fork instrumentation; the child adds
  `+= 211` post-fork. Fails unless parent stays **789** (proving the child-only insert
  did not leak into the parent) and child reads **1000**.
- **test_fork_9** — *Add snippets to both, with ordering:* parent gets `+9` pre-fork
  plus `+11`/`+13` post-fork; child gets `+5`/`+3` post-fork, exercising
  first/last-snippet ordering across a fork. Fails unless parent sums to **40** and
  child to **24**.
- **test_fork_10** — *Synchronous `oneTimeCode` in parent and child* (`+5` parent, `+9`
  child; requires the process to be stopped). Fails if the variable is missing, either
  `oneTimeCode` sets the error flag, or values mismatch: parent **26**, child **30**.
- **test_fork_11** — *Inferior malloc in parent and child:* each process mallocs an
  int, a `oneTimeCode` sets it to 10, then a snippet adds `+3` (parent) / `+7` (child).
  Fails if a malloc returns NULL, lookups/inserts fail, or the malloc'd values aren't
  parent **13** / child **17**.
- **test_fork_12** — *Inferior free in child:* parent mallocs (→10, `+3`); the child
  `free`s the inherited variable. Fails on malloc/lookup failure or parent value ≠
  **13**. Note: the child's `free` has **no direct verification** — only "didn't
  crash" (see Oddities).
- **test_fork_13** — *Inferior free in parent:* both processes malloc (→10); the child
  adds `+5`; the parent `free`s its copy. Fails on malloc/lookup/insert failure or
  child value ≠ **15**; the parent-side free again has no direct check.
- **test_fork_14** — *`oneTimeCode` in both + fork-return semantics:* like fork_10 but
  with no pre-fork preparation; after the synchronous code the mutator must
  `continueExecution` explicitly. Fails if `test_fork_14_global1` is missing, a
  `oneTimeCode` errors, the continue fails, or values mismatch: parent **26**,
  child **30**.

## 1.7 test_mem_* — memory-access instrumentation

Shared pattern: all target the mutatee function `loadsnstores`. Each test builds a
per-architecture reference list of expected `BPatch_memoryAccess` objects, finds access
points via `findPoint(<opcode set>)`, requires the point **count** to equal the
arch-specific constant, `validate()`s every decoded access against the reference list
(any decode disagreement fails), then instruments. All `return SKIPPED` on anything
other than x86/x86_64/power; on x86 they first resolve mutatee variable addresses via
`get_vars_addrs`. `failtest()` marks failure.

- **test_mem_1** — *Load instrumentation* (`BPatch_opLoad`). Fails if `loadsnstores`
  is not found; the load-point count ≠ `nloads` (**41** power / **67** x86 / **75**
  x86_64); `validate` finds a mis-decoded access; or the counting-call instrumentation
  (`instCall`) fails.
- **test_mem_2** — *Store instrumentation* (`BPatch_opStore`). Fails on
  function-not-found; store count ≠ `nstores` (**32** power / **27–43** x86 depending
  on variant / **28** x86_64); validation failure; or instrumentation failure.
- **test_mem_3** — *Prefetch instrumentation* (`BPatch_opPrefetch`). Fails on
  function-not-found; prefetch count ≠ `nprefes` (**0** power / **2** x86 & x86_64);
  validation or instrumentation failure.
- **test_mem_4** — *All three opcode classes together* ("if without else" variant):
  instrument every access point, then re-instrument a `filterPoints(..., 2)` subset.
  Fails on wrong combined access count or **either** `instCall` pass (full set or
  filtered subset) failing.
- **test_mem_5** — *Effective-address snippet:* instrument all accesses with
  `instEffAddr(..., false)` (non-conditional). Fails on function-not-found or
  `instEffAddr` returning failure.
- **test_mem_6** — *Byte-count snippet:* instrument all accesses with a byte-count
  snippet. Fails on function-not-found, wrong access count, or instrument failure.
- **test_mem_7** — *Conditional effective-address snippet* (`instEffAddr(..., true)`).
  SKIPPED except on x86/x86_64 Linux/Windows/FreeBSD. Fails on function-not-found or
  instrument failure.
- **test_mem_8** — *Conditional byte-count snippet.* Fails on function-not-found,
  wrong access count, or instrument failure.

## 1.8 test_stack_* — call-stack walking (`getCallStack`)

Shared pattern: continue the mutatee until it self-stops (`waitUntilStopped`), then
`checkStack(appThread, correct_frame_info, ...)` walks the stack and compares **each
frame** against a hardcoded, arch/OS-`#if`-guarded expected-frames table. Each table
entry carries: must-be-present flag, may-be-absent flag, expected frame type
(`BPatch_frameNormal` / `BPatch_frameSignal` / `BPatch_frameTrampoline`), and expected
function name. Any deviation — missing mandatory frame, wrong type, wrong name — fails.
The Linux/x86 tables include libc/loader frames.

- **test_stack_1** — *Plain `getCallStack`:* expects the walk
  `main → mutateeTest → func1 → func2 → func3 → stop_process_`. Fails if
  `waitUntilStopped` fails or any frame mismatches the table.
- **test_stack_2** — *Stack walk inside a signal handler:* the expected table includes
  `sigalrm_handler` and a frame of type `BPatch_frameSignal`. Fails on wait failure or
  frame mismatch — most importantly, a missing/mistyped signal frame. SKIPPED on
  Windows.
- **test_stack_3** — *Stack walk through instrumentation:* with
  `setInstrStackFrames(true)`, instruments `test_stack_3_func2` at entry, callsite, and
  exit to call func3, then checks the stack at each of the three trip points — each
  walk must contain a `BPatch_frameTrampoline` frame. Fails if func2/func3/points
  aren't found **exactly once each**, or any of the three `checkStack` calls
  mismatches.
- **test_stack_4** — *Walk through an entry-instrumented signal handler:* instruments
  `test_stack_4_sigalrm_handler` entry to call func4; the expected table contains
  **both** a trampoline frame and a signal frame. Fails if handler/func4/entry point
  aren't found exactly once, or the walk mismatches.

## 1.9 test_thread_* — multithreaded mutatees

Shared pattern: a multithreaded mutatee driven via thread-event and user-event
callbacks. Most tests `return SKIPPED` when `supportsUserThreadEvents()` is false.
Callback **registration and removal must both succeed** (a failed
`removeThreadEventCallback`/`removeUserEventCallback` fails the test even if everything
else passed). TIMEOUT while waiting and premature mutatee termination are universal
failures.

- **test_thread_1** — *Runtime-library spinlocks:* runs the threaded mutatee to
  completion with **no instrumentation** (the RT library's locking is what's under
  test). Fails if `waitForStatusChange` fails, or the mutatee doesn't exit normally
  with code 0 (nonzero exit, exit-via-signal, and no-exit all fail).
- **test_thread_2** — *Thread-create callbacks:* register `BPatch_threadCreateEvent`,
  wait for `TEST3_THREADS` creations, then read the mutatee's `test3_threads` tid array
  and cross-check that **every** mutatee tid was reported by some callback. Fails on
  register/continue failure, process death, `getVar` failure on the tid array, any
  unmatched tid, or callback-removal failure.
- **test_thread_3** — *Thread-create callbacks, dead-on-arrival threads:* same
  structure as thread_2, but the mutatee (driven via the `mutateeIdle` variable) spawns
  threads that may already be dead by the time the callback is processed — their tids
  must **still** be reported. Same failure surfaces as thread_2.
- **test_thread_5** — *User-defined message callbacks, multithreaded:* instruments
  createLock/destroyLock/lockLock/unlockLock entries with reporter functions
  (`reportMutexInit/Destroy/Lock/Unlock`) from the test library; the resulting
  user-event messages must validate across all threads. Fails if any function/point is
  missing, any snippet insert fails, callback registration fails,
  `setVar("test_thread_5_idle")` fails, message validation fails, callback removal
  fails, or TIMEOUT.
- **test_thread_6** — *Create + destroy callbacks:* registers both
  `threadCreateEvent` and `threadDestroyEvent`; waits for all creations, checks the
  live-thread count, lets threads finish, then requires **all** `NUM_THREADS` to be
  reported destroyed. Fails on register/remove failure, wrong thread count, any
  callback-side error flag (`error13`), destroyed-count mismatch, or a
  `wait_thread_termination` timeout.
- **test_thread_7** — *Multithreaded trampoline guards:* instruments entries of
  `test_thread_7_level0..level3` so each calls `test_thread_7_level1` — deliberately
  reentrant instrumentation stressing tramp guards under concurrency. Fails if
  create/attach fails, `level1` isn't found exactly once, an insert returns NULL
  (asserted), or the mutatee's exit code is nonzero (the mutatee detects guard
  violations itself).
- **test_thread_8** — *Thread-specific one-time codes:* waits for `NUM_THREADS`
  creations, then per worker thread issues `oneTimeCodeAsync(check_async)` followed by
  a synchronous `oneTimeCode(check_sync)`. Fails (`error_exit`) if the app exits early,
  thread creation times out, expected tids are missing, a tid can't be resolved to a
  `BPatch_thread` (`error15`), callback register/remove fails, or the final check
  `exitCode != 0 || error15 || failed_tests != 0` trips.

## 1.10 Callback & miscellaneous DyninstAPI tests

- **test_callback_1** — *Dynamic-callsite monitoring:* `registerDynamicCallCallback` +
  `monitorCalls` on the dynamic call points in `call2_dispatch`, then `stopMonitoring`.
  Fails when: setup can't find `call2_1..call2_4`; callback registration fails;
  `call2_dispatch` has no subroutine points; the number of **dynamic** points ≠ 3; a
  monitored callee's resolved name doesn't match the expected sequence
  (`expected_fnames`, in order); `stopMonitoring` fails; or TIMEOUT. SKIPPED on Windows
  and for XLC-compiled mutatees (xlc optimizes the dynamic sites away).
- **test_callback_2** — *Async user-event messages from instrumentation:* loads
  libTest12 and instruments entry/exit/callsites of `test_callback_2_call1` with
  reporter functions (`reportEntry`/`reportExit`/`reportCallsite`) that send async user
  messages; validates via `registerUserEventCallback`. Fails when: libTest12
  (`.so`/`_m32`) fails to load; register/remove fails; the function or its points
  aren't found (each `findPoint` must return exactly one point); a reporter function is
  missing; a snippet insert fails; a received message has the wrong **size**; message
  **ordering** isn't `func_entry → N callsites → func_exit`; TIMEOUT; or the mutatee
  exits prematurely.
- **amd64_7_arg_call** — *Function-call snippet with 7 arguments,* forcing stack-passed
  parameters on amd64 (only 6 fit in registers). Mutator-side fails only if
  `amd64_7_arg_call_func` (target), `amd64_7_arg_call` (callee), or the entry point is
  missing. The `insertSnippet` return value is **not** checked (see Oddities); the
  argument values 0..6 are verified mutatee-side.
- **test_write_param** — *Writing function parameters and return values:*
  `BPatch_paramExpr` writes at callsites and callee entry, `BPatch_retExpr` write at
  callee exit. Mutator-side fails if any of `test_write_param_func` / `_call1..._call4`
  is missing or duplicated, or the driver has fewer than 4 subroutine call points.
  Whether the overridden params/return actually took effect is checked mutatee-side.
- **test_snip_remove** — *Deleting a subset of snippets at one point:* insert three
  snippets, delete #1 and #3, keep #2. Fails when: `test_snip_remove_func` / entry /
  variable `test_snip_remove_var` missing; any of the three inserts returns NULL; or
  either `deleteSnippet(sh1)`/`deleteSnippet(sh3)` returns false. Mutatee verifies the
  variable holds only the surviving snippet's contribution.
- **test_reloc** — *Whole-binary function relocation:* relocates **every** function
  (`relocateFunction` inside a begin/finalizeInsertionSet) with no instrumentation, to
  stress the relocation engine. Failing condition is nearly vacuous: it returns PASSED
  even with zero functions and checks no return values (see Oddities) — the effective
  failure mode is a mutator crash or the relocated mutatee misbehaving/crashing at
  runtime. Runtime-library functions are skipped to avoid known self-relocation issues.
- **test_pt_ls** — *End-to-end `parseThat` over a real binary:* runs
  `parseThat /bin/ls /` in CREATE mode (instrument & run) and DISK mode (binary-rewrite
  then execute the rewritten output). Fails when: the runmode is
  unknown (FAILED) — USEATTACH returns SKIPPED; `parseThat`/`parseThat2` returns
  non-PASSED (parse or rewrite failure); or, in DISK mode, `ParseThat::sys_execute` of
  the rewritten `_mutatee_out` binary fails (rewritten-binary crash detector). CPU/MEMORY
  usage stats are parsed from the output when monitoring, but their absence is
  non-fatal. (`ParseThat.C` is the wrapper that locates the external `parseThat` binary
  via `$PATH` or `$DYNINSTAPI_RT_LIB/../bin` and fork/execs it; it logs if the binary
  can't be resolved.)
- **snip_change_shlib_var** — *Instrumentation writes a variable inside a shared
  library:* at `scsv1` entry assign 777 to the shlib variable `snip_change_shlib_var`;
  at exit call the shlib function `check_snip_change_shlib_var` and store its verdict
  into `gv_scsv1`. Fails when: `loadLibrary` of libtestA or libtestB fails; `scsv1`,
  the check function, either variable, or the entry/exit points are missing; or either
  insert returns false. The 1/0 verdict of the check function is evaluated mutatee-side.
- **snip_ref_shlib_var** — *Snippets read shared-library variables of several types*
  (scalars plus an array element accessed via `BPatch_ref`), copying each into a
  mutatee global. Fails when: libtestA/B don't load; `srsv1` or its entry point is
  missing; any source variable `snip_ref_shlib_varN` or destination `gv_srsvN` is
  missing (the lookup helper returns NULL ⇒ FAILED); or the single combined
  `BPatch_sequence` insert fails. The var6 case is disabled with `#if 0` (see
  Oddities).
- **init_fini_callback** — *`insertInitCallback`/`insertFiniCallback`* on both the
  a.out and a loaded libtestA (module init/fini instrumentation). `executeTest` fails
  when: `entry_call`/`exit_call` reporter functions are missing; libtestA fails to
  load; or any of the four insertion flags (init/fini × a.out/libtestA) is false.
  **This test is effectively broken**: its `postExecution()` returns FAILED on *every*
  path — both branches of its `strncmp` check return FAILED — so it cannot pass as
  written (see Oddities).

---

# 2. InstructionAPI tests (`src/instruction/`)

Shared framework (`instruction_comp.C`/`.h`), used by every test below:

- `verify_read_write_sets(insn, expectedRead, expectedWritten)` — calls
  `insn.getReadSet()`/`getWriteSet()` and FAILS if: the set sizes differ from expected;
  the element-by-element comparison (ordered by `shared_ptr_lt`, compared with
  `indirect_equal`) mismatches; any `RegisterAST::Ptr` is NULL; or
  `insn.isRead()`/`isWritten()` disagrees with set membership.
- `verifyCFT(cft, expectedDefined, expectedValue, expectedType)` — evaluates a
  control-flow target and FAILS if `Result.defined`, `Result.type`, or the converted
  integer value doesn't match.
- Any instruction in a test's buffer failing to decode (`insn.isValid()` false) fails
  the test, as do the "size mismatch" guards between the instruction list and the
  expected-results list.

- **aarch64_cft** — *AArch64 control-flow-target extraction:* decodes 9 hand-encoded
  branches (B, BR, RET, CBZ, B.NE, TBZ, TBNZ, BL, BLR; buffers endian-swapped before
  decoding). Fails when: a decode is invalid; the CFT count (`cft_begin..cft_end`) ≠
  expected; a target is NULL; the computed target after binding **pc=0x400** and
  **x12=0x90** has wrong value/definedness/type; or any of the four per-target flags
  (`isCall`/`isIndirect`/`isConditional`/`isFallthrough`) mismatches.
- **aarch64_decode_ldst** — *AArch64 load/store decode + register sets* over **136**
  instructions (literal/pre/post-index/register-offset/exclusive/pair/unscaled/
  unprivileged loads & stores, prfm, csinc/csinv/csneg, bitfield/mov-immediate forms).
  Fails when: any decode is invalid, or a read/write set mismatches — typical causes:
  wrong base/index register, wrong operand-size register (x1 vs w1), missing writeback
  register on pre/post-index forms, missing nzcv for conditional selects, or sp/xzr
  mishandling.
- **aarch64_simd** — *AArch64 SIMD/NEON decode + register sets* over **131**
  instructions (reductions saddlv/smaxv..., dup/ins/smov/umov, ext/movi/mvni/bic,
  zip/trn/uzp, shift-by-immediate, saturating multiply-accumulate, tbl/tbx,
  widening/narrowing arithmetic, ld1–ld4/st1–st4 structure loads/stores). Fails when:
  any decode is invalid or a register set mismatches — element-size register class
  (d/q/s/h/b), structure-list registers, post-index writeback base, or the `hq*`
  half-quad aliasing.
- **fucompp** — *x87 floating-point compares:* 24 instructions
  (fcom/fcomp/fcompp, fcomi/fcomip, fucomi/fucomip, fucom/fucomp/fucompp, with memory
  and st(i) operands), decoded as `Arch_x86`. Fails on decode failure or wrong sets:
  st0/st(i) read membership, memory-operand base (ebp) capture, and the distinction
  that plain fucom variants expect an **empty** write set while fucomp writes st0.
- **mov_size_details** — *Operand size reporting:* 5 x86 instructions (mov ax/gs,
  mov eax/ebp, movddup xmm, add eax imm, fcomp); checks
  `insn.getOperand(i).getValue()->size()` for both operands. Fails on decode failure
  or any size mismatch. Note: the fcomp case expects `{8, 8}` with an in-code comment
  admitting the correct answer is 80 bits (see Oddities).
- **power_cft** — *PowerPC branch CFT extraction:* I-form b/ba/bl/bla, B-form
  conditional bc/bca/bcl/bcla, XL-form bclr/bcctr, plus fallthrough targets. Fails
  when: decode invalid; **zero** CFTs found; `num_targets_seen ≠
  num_targets_expected`; a target value/type mismatches after binding **pc=0x400,
  ctr=44, lr=0x200**; a per-target call/conditional/indirect/fallthrough flag is wrong;
  or `isBranch()`/`isReturn()` disagrees with expectations. (bctar cases are commented
  out as undecodable — see Oddities.)
- **power_decode** — *PowerPC decode + register sets:* 27 instructions decoded as
  `Arch_ppc32` (add/add./addo, fadd/fadd., addi, fcmpu, mtcrf, mtfsf,
  lwz/lwzu/lwzx/lwzux, rlimi, fp multiplies, bdnz/bclr/bctr). Fails on decode failure
  or set mismatch — cr0 written by "." record forms, xer by "o" overflow forms, fpscw
  side effects, the writeback register on update-form loads, ctr/lr/pc for branches.
  Additionally fails if the final standalone check `insn.readsMemory()` returns false
  for lhzux.
- **ppc64_decode_test** — *Bulk ppc64 decode coverage tool* (standalone `main()`, not a
  registered mutator): decodes instructions listed in three input files
  (hexcode/opcode-name/binary), strips `insn.format()` to a mnemonic, and compares
  against the expected-name file, tallying "not implemented" and "third level opcode"
  counts into `test_result.txt`. It has **no PASS/FAIL** — a mismatch is counted, not
  asserted; it aborts early only on file-open/format errors (see Oddities).
- **test_instruction_bind_eval** — *Operand binding & evaluation:* decodes
  `call [8*EAX + ECX + 0xDEADBEEF]` for both `Arch_x86` and `Arch_x86_64`; exercises
  `getControlFlowTarget()`, incremental `bind()` of EAX (=3) and ECX (=5),
  `getSubexpressions()`, and `eval()`. Fails when: decode invalid; no CFT; the CFT — a
  memory dereference — becomes *defined* (it must stay **undefined** even after both
  binds, since the memory contents are unknown); a `bind()` returns false;
  subexpression count ≠ 1 (the single dereference child); or the inner memory-reference
  expression doesn't evaluate to `0xDEADBEEF + 3*8 + 5` as a u32.
- **test_instruction_farcall** — *Far-call decode smoke test:* the 32-bit x86 far call
  `CALL 0504030201` (opcode 0x9A) must decode. Fails **only** if `decode()` yields an
  invalid instruction — no operand/CFT/register checks at all.
- **test_instruction_profile** — *Bulk decode robustness over a real libc:* opens the
  platform libc, iterates its code regions, linearly decodes every instruction, and
  counts valid + control-flow instructions. Fails **only** if libc cannot be opened for
  parsing; SKIPPED on unknown architectures; otherwise always passes — it is a
  no-crash/robustness exercise, not a correctness assertion (see Oddities).
- **test_instruction_read_write** — *x86/x86_64 read/write register sets:* 11
  instructions run under both arches (add imm, push, jz, call, clc, add al,
  mov mem-imm, mov byte, movddup, haddpd, lea) plus one 64-bit-only case
  (`mov [rbp-0x3c], r8d`). Fails on decode failure or set mismatch — flag-register
  writes for arithmetic, implicit sp/ip read/write for push/jz/call, stores writing
  **no** registers, lea reading its base register without a memory dereference, and
  the 64-bit case reading exactly {rbp, r8d} with an empty write set.

---

# 3. ProcControlAPI tests (`src/proccontrol/`)

Shared framework (`proccontrol_comp.C`/`.h`, `pcontrol_mutatee_tools.h`):
`ProcControlComponent` launches or attaches to N mutatee processes (per `createmode`:
launch/attach/fork) and communicates with each over a **socket**. The mutator holds
`comp->procs` (vector of `Process::ptr`) and `comp->pset` (a `ProcessSet`);
`comp->num_processes`/`num_threads` define expected counts. Mutator and mutatee exchange
fixed-code control messages — `SENDADDR_CODE` (mutatee reports a function/data address),
`SYNCLOC_CODE`, `HANDSHAKE_CODE`, `ALLOWEXIT_CODE`, fork/thread info — via
`send_message`/`recv_message` and broadcast variants. Events arrive through
`Process::registerEventCallback` handlers pumped by `block_for_events()` /
`poll_for_events()`.

**Universal failing conditions** (assumed for every test below): a received control
message carries the wrong code; a send/recv fails; an API call (`continueProc`,
`stopProc`, `addBreakpoint`, ...) returns false; or an event callback sets its global
error flag (`myerror`/`had_error`/`cb_error`).

- **pc_launch** — *Launch/attach smoke test:* just resume all created processes. Fails
  only if `continueProc()` returns false for any process.
- **pc_breakpoint** — *Software breakpoints* (`Breakpoint::newBreakpoint` +
  `addBreakpoint`): verifies hit addresses, `getData()` payload indices, and per-thread
  hit counts. Fails when, in the callback: the event isn't a breakpoint; it carries ≠ 1
  breakpoint object; the `getData()` index is missing/invalid; the breakpoint pointer
  doesn't match the mutator's `bps[][]` table; or the hit address ≠ expected. Fails at
  the end if the total hits never reach `NUM_BREAKPOINTS × NUM_BREAKPOINT_SPINS × procs
  × threads`, or any single thread's count ≠ `NUM_BREAKPOINT_SPINS × NUM_BREAKPOINTS`.
- **pc_hw_breakpoint** — *Hardware breakpoints* across read/write/execute modes and
  sizes, including slot management via `numHardwareBreakpointsAvail`. Fails when:
  `newHardwareBreakpoint` returns NULL; add/remove/stop/continue fails; no slot can be
  freed when needed; a callback carries ≠ 1 breakpoint or an unrecognized pointer; a
  breakpoint's `times_hit` ≠ its mode-specific expected count; it fires **before**
  insertion; or it fires again **after** removal.
- **pc_singlestep** — *Single-stepping a subset of threads* (`setSingleStepMode`),
  including interaction with a coincident software breakpoint. Fails when: a
  single-step event arrives on a thread **not** in single-step mode; a function is
  stepped twice; a stepping thread records zero steps; the expected function sequence
  isn't stepped in order; the designated stop function is stepped **past**; the
  breakpoint/step ordering at the coincident address is wrong; or a non-stepped thread
  recorded any steps.
- **pc_irpc** — *Inline RPCs* (`IRPC::createIRPC`/`postIRPC`/`runIRPCSync`),
  exhaustively over the matrix {allocation mode} × {post time} × {post to
  process/thread} × {sync/async/postsync} × {stopped/running start state}. Fails
  (`myerror`) when: any post/malloc/free/read/write/continue fails; a callback receives
  an unrecognized or already-completed IRPC; an RPC runs **out of posting order**; an
  unposted RPC runs; the callback's thread disagrees with the posting thread; the PC
  register can't be read in the callback; `handleEvents` returns with sync RPCs still
  pending; or the final read-back accumulator `val ≠ (num_threads+1) × NUM_IRPCS`
  (some RPC bodies never executed).
- **pc_fork** — *Fork events:* child `Process` identity, breakpoint inheritance, and
  exit tracking of forked children. Fails when: child == parent; a child pid is seen
  twice; child/parent library counts differ; or, per child (checked against the
  mutatee's `forkinfo` messages): `parent` pointer wrong, pid mismatch, the child never
  hit the inherited breakpoint (or hit it twice), didn't exit, isn't marked exited,
  exit code ≠ `EXIT_CODE` (4), or mutator/mutatee disagree on `is_threaded`.
- **pc_fork_exec** — *Fork-then-exec:* Exec event delivery and library-set changes
  across exec. Fails when: child == parent or duplicated; parent/child library counts
  differ or the child lacks libtestA after fork; the **exec'd** process still has
  libtestA (the pre-exec image leaked); total callbacks ≠
  `num_processes × (num_threads+1)`; a process delivers no exit callback; the exec
  path doesn't contain `pc_exec_targ`; or exit code ≠ 4.
- **pc_detach** — *Full detach:* after `Process::detach()`, **no** events may be
  delivered. Fails when: continue/detach returns false; a Signal callback fires
  post-detach (`signal_error`); or the post-detach sync exchange with the mutatee fails
  (`sync_points[j].code != SYNCLOC_CODE`) — i.e. the detached mutatee didn't keep
  running normally on its own.
- **pc_temp_detach** — *Temporary detach/reattach*
  (`temporaryDetach()`/`reAttach()`): the mutatee keeps generating events while
  detached; after reattach the mutator reads the mutatee's event counter. Fails when:
  continue/temporaryDetach/reAttach fails; **any** event is received while detached
  (`poll_for_events()` true, or the signal callback fires while `not_expecting_event`);
  reading the counter fails; or the counter is **0** — the mutatee didn't actually run
  freely, meaning the detach never really released it.
- **pc_terminate** — *`terminate()` on a running process* (via
  `pc_terminate_common.C`, `should_stop() == false`): termination must be **silent**.
  Fails when: `terminate()` returns false; any Exit (pre or post) or Crash callback
  fires; the post-terminate sync broadcast still succeeds (the mutatee survived); or a
  process isn't marked terminated / is wrongly marked exited / reports an exit code /
  is marked crashed.
- **pc_terminate_stopped** — *Terminate an already-stopped process* (same common code,
  `should_stop() == true`): identical conditions plus a failure if the pre-terminate
  `stopProc()` returns false.
- **pc_thread** — *Thread/LWP create & destroy events:* validates TID/LWP uniqueness,
  start function, stack base/size, TLS pointer, initial-thread identity, and
  cross-checks everything against the mutatee's `threadinfo` messages
  (`checkThreadMsg`: pid, TID, stack range, initial function, TLS range). SKIPPED when
  `supportsUserThreadEvents()` is false. Fails when: a thread lacks user-thread
  info/TID after its create event; duplicate TID/LWP/stack/TLS across threads; the LWP
  callback doesn't precede the user-thread callback; start-function/stack/TLS missing
  where the platform promises them; a destroy event fires twice or for an unknown
  thread; a process exits before all thread-terminate events arrived; or destroy
  callbacks never reach `num_processes × (num_threads − 1)`.
- **pc_thread_cont** — *Per-thread continue* (`Thread::continueThread`): stop the whole
  process, then release threads one at a time to exit, verifying destroy events per
  thread. Fails when: any process/thread stop/continue fails; the thread-pool size ≠
  `num_threads + 1`; the initial thread isn't first in the iterator or appears more
  than once; the thread list is too short; a destroy callback arrives for a thread not
  in the expected pre/post thread/LWP sets (or for an event type the platform doesn't
  deliver); or the `HANDSHAKE_CODE` exchange fails.
- **pc_groups** — *`ProcessSet`/`AddressSet` bulk operations:* group stop/continue;
  uniform, non-uniform, and aggregated reads & writes; group malloc/free; group
  add/remove breakpoint. Fails when: any set operation returns false; a read-back
  mismatches the sequence **4 → 8 (after write) → 16 (in group-malloc'd memory)**; an
  aggregated read doesn't collapse to exactly 1 distinct result; the process set
  returned by an operation differs from `pset` (set-difference non-empty); the
  group-malloc result-set size ≠ pset size; the group-breakpoint hit count ≠
  `(num_threads+1) × num_processes`; or a breakpoint still fires after `rmBreakpoint`.
- **pc_library** — *Library enumeration and load/unload ordering:* static check
  (executable and libc present via `getLibraryByName`) plus dynamic load/unload of
  libtestA/libtestB. Fails when: `getLibraryByName` returns an inconsistent pointer; a
  newly added library isn't in the library pool, or a removed one still is; an
  unexpected breakpoint fires; fewer processes than expected reported library events;
  testA/testB was never loaded or never unloaded; or the event order isn't exactly
  **loadA(0), loadB(1), unloadB(2), unloadA(3)**.
- **pc_addlibrary** — *`Process::addLibrary()`:* injecting libtestA must produce
  exactly one Library event per process. Fails when: `addLibrary` returns false; no
  Library callback arrives for a process; the callback is a non-library event, reports
  a *removed* library, is empty, doesn't contain `libtestA`, or fires **twice**; or an
  unexpected Breakpoint/RPC callback is delivered.
- **pc_mem_perm** — *Memory permission get/set*
  (`getMemoryAccessRights`/`setMemoryAccessRights`) over R/W/X combinations.
  **Hard-coded to `return SKIPPED`** — the test is disabled (see Oddities). Its dormant
  body would fail on malloc/get/set/free failure or when read-back rights differ from
  those set.
- **pc_stat** — *Sampling loop* (×10) over a ProcessSet: stop everything,
  `getAllRegisters`, `CallStackUnwinding::walkStack`, refresh libraries/LWPs, continue.
  Fails when: the initial spin-address set size ≠ num_processes; stop/continue fails;
  the register-set count ≠ `num_processes × (num_threads+1)`; a register set lacks the
  **stack pointer**; the readMemory count is wrong; the stackwalk's begin/frame/end
  thread sets don't each cover the full thread set; or
  refreshLibraries/refreshLWPs/write fails.
- **pc_tls** — *Thread-local storage*
  (`readThreadLocalMemory`/`writeThreadLocalMemory`) on TLS variables in both the
  executable and libtestA, iterated per thread. Fails when: a required symbol isn't
  found via the symbol reader; a TLS read returns the wrong per-iteration value
  (int == iteration, long == −iteration) or a read/write returns false; or any
  thread's completed-iteration count ≠ `NUM_ITERATIONS` (8).

---

# 4. SymtabAPI tests (`src/symtab/`)

Shared framework (`symtab_comp.C`/`.h`): during `group_setup`, `SymtabComponent` calls
`Symtab::openFile(symtab, group->mutatee)` on the compiled mutatee binary and hands the
`Symtab*` to each test. **Setup failure (file can't be opened) fails every test in the
group.**

- **test_lookup_func** — *Function enumeration & detail.* Fails when:
  `getAllSymbolsByType(ST_FUNCTION)` returns false or an empty vector;
  `findFunctionsByName("lookup_func")` returns 0 or **more than 1** match; the function
  is NULL or has a zero offset; `getLocalVariables` fails or the count ≠ 1; the local's
  name ≠ `my_local_var`; `getParams` fails or the count ≠ 1; the param's name ≠
  `my_param`.
- **test_lookup_var** — *Global-variable lookup.* Fails when:
  `findVariablesByName(vars, "lookup_var")` returns false/empty, or returns **more than
  one** match. (Class/factory misnamed `test_lookup_func` — see Oddities.)
- **test_local_var_lookup** — *Local-variable lookup smoke test.* Fails when:
  `findFunctionsByName("test_local_var_lookup_mutatee")` returns false, or
  `findLocalVariable(vars, "local_var_lookup")` returns false. (No null/size check on
  `funcs[0]` before dereference — see Oddities.)
- **test_local_var_locations** — *DWARF location lists in range.* Fails when:
  `findFunctionsByName("test_local_var_locations_mutatee")` fails; any of the variables
  `local_var_locations`, `secondVariable`, `thisIsReference` isn't found via
  `findLocalVariable`; a variable has **no** location lists; or any location's
  lowPC/hiPC falls outside `[funcOffset, funcOffset + size]`.
- **test_line_info** — *DWARF line info.* Fails when: `getAllModules` is empty; for the
  modules expected to have statements (`mutatee_util.c`, `solo_mutatee_boilerplate.c`,
  `mutatee_driver.c`) any statement has an empty file name, line 0, NULL/−1
  start/end address, or endAddr < startAddr; `findFunctionsByName("test_line_info_func")`
  ≠ 1; `getParams` fails or param count ≠ 1; the param's line number ≠ **1000**;
  `getLocalVariables` fails or count ≠ 3; or the three locals' line numbers aren't
  **2000, 2001, 2002** in order. (Non-Linux and PGI compilers short-circuit to PASSED
  after the param check.)
- **test_module** — *Module metadata & index consistency.* Fails when: `getAllModules`
  is empty; any module is NULL or malformed (empty `fileName`, empty `fullName`, NULL
  `exec()` parent); a module can't be re-found via `findModulesByName(fileName)`; or
  can't be re-found via `findModuleByOffset(mod->addr())`.
- **test_type_info** — *Type-system verification* over std types, builtin types, and
  all module types. Fails when: `getAllstdTypes`/`getAllbuiltInTypes` is NULL/empty;
  any type has a zero ID, an unknown/NULL data class, or a variety-specific defect
  (empty enum, unnamed enum element, NULL pointer/array/typedef constituent, subrange
  low > high, array low > high, NULL field type). Specific named checks: enum `enum1`
  values must be **ef1_1=20, ef1_2=40, ef1_3=60, ef1_4=80**; union `my_union` fields
  (`my_float`:float, `my_int`:int); struct `mystruct` fields `elem1..elem4` with
  expected (or listed alternate) type names; typedef `int_alias_t` → int;
  `int_array_t` an array of int with range **[0, 255]**; `my_intptr_t` a typedef of
  pointer-to-int. DWARF base-type property structs `integral_t`/`signed_integral_t`/
  `unsigned_integral_t`/`floating_point_t` must have exact field counts (6/5/5/6) with
  matching `typeScalar` properties (is_integral, is_signed/unsigned, is_boolean,
  signed/unsigned_char, is_floating_point, is_complex_float). Finally `got_all_types()`
  fails the test if any type-variety category was never encountered at all
  (subrange/typedef exempted on Windows).
- **test_exception** — *C++ exception-handler blocks.* Fails when:
  `symtab->getAllExceptions()` returns false or the vector is empty. SKIPPED on
  Windows. The stricter count check against `NUM_EXPECTED_EXCPS` (3) is disabled with
  `#if 0` (see Oddities).
- **test_relocations** — *Function-binding/relocation table.* Fails when:
  `getFuncBindingTable` returns false/empty; no `libc.so` can be located and opened
  from the standard library directories; **none** of the expected libc relocation names
  (printf, fprintf, memcpy, strcmp, fopen, ...) both appears in the relocations *and*
  resolves via `libc->findFunctionsByName` (`found_one` stays false); or the count of
  found libtestA relocations (`relocation_test_function1`, `relocation_test_function2`)
  ≠ 2. SKIPPED on Windows.
- **test_add_symbols** — *Symtab mutation & round-trip:* add a mangled name to a
  function, add a brand-new symbol, `emit` the rewritten object under `./binaries/`,
  re-open it, and re-look-up. Fails when: `findFunctionsByName("add_sym_func")` returns
  0 or > 1; the function has zero offset; `addMangledName("add_sym_func_newname")`
  fails; the symbol count after the adds isn't oldCount+1 then +2; `addSymbol`
  fails; mkstemp/emit/openFile of the rewritten file fails; or, in the re-read file,
  `findSymbol("add_sym_newsymbol")` ≠ 1 match, its offset ≠ (page adjustment + original
  offset), or `findFunctionsByName("add_sym_func_newname")` ≠ 1 match.
- **test_anno_basic_types** — *Annotatable framework:* size guarantees plus add/get/
  remove round-trips for all basic types (int, char, short, long, float, double,
  signed/unsigned variants) under both type-derived and custom names
  ("auxname1".."auxname10"). Fails when: `sizeof(TestClassSparse) != sizeof(TestClass)`
  (sparse annotations must add **zero** bytes); `sizeof(TestClassDense) !=
  sizeof(TestClass) + sizeof(void*)` (dense must add exactly one pointer); an
  annotation add fails; a get returns NULL or a value ≠ the one stored; a remove
  fails; a removed annotation is still retrievable; or the custom-named annotation set
  doesn't survive removal of the type-named set.

---

# 5. StackwalkerAPI programs (`src/stackwalker/`)

These are drivers/demos rather than pass/fail component tests (see Oddities).

- **test_basic** — *Interactive stackwalk driver:* builds `Walker` objects for
  `-self`, `-attach <pid>`, or `-create <exec>` targets, then every 5 seconds (or on
  stdin input) walks each thread's stack via `walkStack`, printing frame names, RA/FP
  values, RA/FP/SP locations, and — when built with symtab support — local-variable
  values via `getFunctionForFrame`/`getLocalVariables`/`getLocalVariableValue`. **No
  hard PASS/FAIL**: operational failure means no walker could be created (usage
  message/abort), `walkStack` yields no or garbage frames, `getFunctionForFrame`
  returns NULL (printed as `<no func>`), or a variable's value can't be retrieved
  (printed as `<no value>`).
- **stack_sampler** — *Sampling driver:* attaches to or creates a process via
  ProcControlAPI, sets a breakpoint, then repeatedly samples every thread's stack with
  `Walker::walkStack`, counting good vs. attempted walks. Returns −1 only if
  `getAvailableThreads` fails; per-thread walk failures are logged (with
  `Stackwalker::getLastErrorMsg()`) and counted as bad rather than failing anything.
  Note the hard-coded breakpoint address `0x804843e` — 32-bit x86 only (see Oddities).
- **stack.c / while1.c** — *Mutatees:* `stack.c` provides a known deep call chain
  (`main → func1 → func2 → func3`, recursing then spinning in `while(1)`) so a walker
  can verify expected frames; `while1.c` is a minimal spinning `main` for
  attach/self-walk targets.

---

# 6. Oddities, gaps, and known quirks

Things noticed while reading the tests that affect how much you can trust a PASS, or
that explain surprising results.

## Broken or disabled tests

1. **`init_fini_callback` cannot pass as written.** Its `postExecution()` contains a
   `strncmp` whose *both* branches `return FAILED` — every execution path through
   post-execution reports failure regardless of what the instrumentation did. Until
   that function is fixed, any FAILED from this test says nothing about
   `insertInitCallback`/`insertFiniCallback` actually being broken.
2. **`pc_mem_perm` is hard-disabled.** The first line of its `executeTest` is
   `return SKIPPED`, so memory-permission get/set
   (`getMemoryAccessRights`/`setMemoryAccessRights`) has **zero** live coverage. The
   dormant body (R/W/X permission round-trips) is intact below the early return.
3. **`snip_ref_shlib_var`'s var6 case is `#if 0`'d out** — one of the shared-library
   variable types it was written to cover is silently not tested.
4. **`test_exception`'s count check is `#if 0`'d out.** It only verifies that
   `getAllExceptions()` returns a non-empty vector; the stricter check that exactly
   `NUM_EXPECTED_EXCPS` (3) handler blocks are found is disabled. A binary with the
   wrong number of exception blocks still passes.
5. **`power_cft`'s bctar cases are commented out** as undecodable — the XL-form
   branch-conditional-to-TAR instruction is deliberately excluded from CFT coverage.
6. **test5_9 carries a comment that additional test5 subtests exist but were never
   enabled** — C++ coverage stops at derivation.
7. **Numbering gaps:** there is no `test1_35` and no `test2_11`; these are skipped
   numbers, not missing files.

## Tests that pass more easily than their names suggest

8. **`test_reloc` is nearly unconditional PASS.** It relocates every function but
   checks no return values from `relocateFunction`/`finalizeInsertionSet`, and returns
   PASSED even when `getProcedures()` is empty. Its only real failure mode is a crash
   (mutator or relocated mutatee). A green `test_reloc` means "didn't crash," not
   "relocation verified correct."
9. **`test_instruction_profile` asserts almost nothing.** It fails only if libc can't
   be *opened*; every decode result is merely counted. It is a robustness/no-crash
   exercise — decoder regressions that produce wrong-but-valid instructions pass.
10. **`test_instruction_farcall` checks only decode validity** — no operand, CFT, or
    register checks. It guards against "far call fails to decode at all" and nothing
    else.
11. **`test3_4` and `test3_5` only fail if `processCreate` returns NULL.** The
    sequential exit/abort monitoring loops always fall through to PASSED — a mutatee
    that hangs or exits abnormally is not caught by the assertions (only,
    incidentally, by the harness timeout).
12. **`amd64_7_arg_call` doesn't check its `insertSnippet` return value.** A silently
    failed insertion would surface only as the mutatee-side argument check failing —
    the mutator would still report its own phase as fine.
13. **`test_fork_12`/`test_fork_13`: the inferior `free` under test has no direct
    verification.** Only the *other* process's variable value is checked; a `free`
    that silently corrupts or no-ops passes as long as nothing crashes.
14. **`test2_12` doesn't assert the `usesTrap_NP()` result** — it only exercises the
    call path; any return value passes.
15. **The stackwalker programs (`test_basic`, `stack_sampler`) have no PASS/FAIL at
    all.** They are interactive/diagnostic drivers. `ppc64_decode_test` likewise is a
    coverage-tally tool writing `test_result.txt`, not an asserting test.

## Encoded-wrong or platform-fragile expectations

16. **`mov_size_details` enshrines a known-wrong value.** The fcomp case expects
    operand sizes `{8, 8}` with an in-code comment that the correct answer is **80
    bits** (x87 extended precision). If the decoder is ever *fixed* to report 10
    bytes, this test will start failing — that failure would be the fix, not a
    regression.
17. **`stack_sampler` hard-codes breakpoint address `0x804843e`** — a 32-bit x86 text
    address. On any other platform/binary layout the breakpoint lands on garbage.
18. **`ppc64_decode_test`'s usage string mentions 4 arguments but the code consumes
    3** (hexcode file, opcode-name file, binary file).
19. **The `test_stack_*` expected-frame tables are heavily `#if`-guarded per
    arch/OS** (including specific libc/loader frame names on Linux/x86). New
    platforms, libc versions, or frame-layout changes typically require table updates
    rather than indicating real stack-walker breakage.
20. **Platform/compiler skips are pervasive and silent-ish:** many DyninstAPI tests
    return SKIPPED for Fortran mutatees (test1_24/26/27/28/30/33/37/38/40/41), for
    XLC-built mutatees (test1_40, test_callback_1 — xlc optimizes the indirect-call
    dispatch away), on Windows (fork family, test3_6, test4_2–4_4, test_stack_2,
    test_callback_1, symtab test_exception/test_relocations), and outside
    x86/x86_64/power (the whole test_mem family). A green run on one platform says
    little about these paths elsewhere.
21. **`test2_4` is SKIPPED when run as root** — root can attach to pid 1, so the
    expected-failure test would spuriously fail; CI running as root silently loses
    this negative test.

## Naming / hygiene quirks

22. **`test_lookup_var`'s class and factory are misnamed `test_lookup_func`** — the
    file tests variable lookup but its symbols say function lookup. Harmless, but
    confusing when reading logs or registration tables.
23. **`test_local_var_lookup` dereferences `funcs[0]` without checking the vector is
    non-empty** — if `findFunctionsByName` succeeds but returns an empty vector, the
    test crashes rather than failing cleanly.
24. **`test1_18`'s expected `readValue` differs by language** — 42 for C mutatees but
    0 for Fortran — worth knowing before "fixing" a Fortran-side mismatch.
25. **Helper files live alongside tests:** `test_lib_test7.C` (fork-family messaging
    and `verifyProcMemory`), `test_lib_test9.C` (`sleep_ms`), `cpp_test.C`
    (mutatee-side C++ recorder), `Callbacks.C`/`Process_data.C`/`ParseThat.C`
    (framework support) contain no test logic of their own despite test-like names.

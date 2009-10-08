#include "cpp_test.h"
#include "mutatee_util.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

overload_op_test test5_3_test3;
static int passed = 0;

/* Function definitions follow */

void overload_op_test::func_cpp()
{
   overload_op_test test;
   ++test;
}

void overload_op_test::call_cpp(int arg)
{
   if ( arg == 3 ) {
     passed = 1;
     logerror("Passed test #3 (overload operator)\n");
   } else {
     logerror("**Failed** test #3 (overload operator)\n");
     logerror("    Overload operator++ return wrong value\n");
   }
}

int overload_op_test::operator++()
{
  return (cpp_test_util::CPP_TEST_UTIL_VAR);
}

int test5_3_mutatee() {
  test5_3_test3.func_cpp();
  // FIXME Make sure the error reporting works
  // I need to have this guy call test_passes(testname) if the test passes..
  if (1 == passed) {
    // Test passed
    test_passes(testname);
    return 0;
  } else {
    // Test failed
    return -1;
  }
}
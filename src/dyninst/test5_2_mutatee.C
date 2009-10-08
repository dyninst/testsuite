#include <stdio.h>

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

overload_func_test test5_2_test2;
static int passed = 0;

/* Function definitions follow */

void overload_func_test::func_cpp()
{
   call_cpp("test overload function");

   call_cpp(2);

   call_cpp(2, 2.0);
}


void overload_func_test::call_cpp(const char * arg1)
{
  DUMMY_FN_BODY;
}


void overload_func_test::call_cpp(int arg1)
{
  DUMMY_FN_BODY;
}


void overload_func_test::call_cpp(int arg1, float arg2)
{
  DUMMY_FN_BODY;
}

void overload_func_test::pass() {
  passed = 1;
}

int test5_2_mutatee() {
  test5_2_test2.func_cpp();
  if (1 == passed) {
    // Test passed
    test_passes(testname);
    return 0;
  } else {
    // Test failed
    return -1;
  }
}
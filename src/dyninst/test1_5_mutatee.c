/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"
#include <limits.h>

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_5_func2();
int test1_5_func1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_5_globalVariable5_1 = 0;
int test1_5_globalVariable5_2 = 1;
int test1_5_globalVariable5_3 = 1;
int test1_5_globalVariable5_4 = 1;
int test1_5_globalVariable5_5 = 0;
int test1_5_globalVariable5_6 = 1;
int test1_5_globalVariable5_7 = 0;
int test1_5_globalVariable5_8 = 1;
int test1_5_globalVariable5_9 = 1;
int test1_5_globalVariable5_10 = 1;
int test1_5_globalVariable5_11 = 1;
int test1_5_globalVariable5_12 = 1;
int test1_5_globalVariable5_13 = 1;
int test1_5_globalVariable5_14 = 1;
int test1_5_globalVariable5_15 = 1;
int test1_5_globalVariable5_16 = 0;
int test1_5_globalVariable5_17 = 1;


/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

int test1_5_func1() {
  int retval;
  test1_5_func2();

  if (!(test1_5_globalVariable5_1
      || test1_5_globalVariable5_2
      || test1_5_globalVariable5_3
      || test1_5_globalVariable5_4
      || test1_5_globalVariable5_5
      || test1_5_globalVariable5_6
      || test1_5_globalVariable5_7
      || test1_5_globalVariable5_8
      || test1_5_globalVariable5_9
      || test1_5_globalVariable5_10
      || test1_5_globalVariable5_11
      || test1_5_globalVariable5_12
      || test1_5_globalVariable5_13
      || test1_5_globalVariable5_14
      || test1_5_globalVariable5_15
      || test1_5_globalVariable5_16
      || test1_5_globalVariable5_17
      )) {
    logerror("Passed test #5 (if w.o. else)\n");
    retval = 0; /* Test passed */
  } else {
    logerror("**Failed** test #5 (if w.o. else)\n");
    retval = -1; /* Test failed */
    if (test1_5_globalVariable5_1) logerror("\t ifexpr5_1 failed\n");
    if (test1_5_globalVariable5_2) logerror("\t ifexpr5_2 failed\n");
    if (test1_5_globalVariable5_3) logerror("\t ifexpr5_3 failed\n");
    if (test1_5_globalVariable5_4) logerror("\t ifexpr5_4 failed\n");
    if (test1_5_globalVariable5_5) logerror("\t ifexpr5_5 failed\n");
    if (test1_5_globalVariable5_6) logerror("\t ifexpr5_6 failed\n");
    if (test1_5_globalVariable5_7) logerror("\t ifexpr5_7 failed\n");
    if (test1_5_globalVariable5_8) logerror("\t ifexpr5_8 failed\n");
    if (test1_5_globalVariable5_9) logerror("\t ifexpr5_9 failed\n");
    if (test1_5_globalVariable5_10) logerror("\t ifexpr5_10 failed\n");
    if (test1_5_globalVariable5_11) logerror("\t ifexpr5_11 failed\n");
    if (test1_5_globalVariable5_12) logerror("\t ifexpr5_12 failed\n");
    if (test1_5_globalVariable5_13) logerror("\t ifexpr5_13 failed\n");
    if (test1_5_globalVariable5_14) logerror("\t ifexpr5_14 failed\n");
    if (test1_5_globalVariable5_15) logerror("\t ifexpr5_15 failed\n");
    if (test1_5_globalVariable5_16) logerror("\t ifexpr5_16 failed\n");
    if (test1_5_globalVariable5_17) logerror("\t ifexpr5_17 failed\n");
  }
  return retval;
}

/*
 * Start of Test #5 - if w.o. else
 *	Execute two if statements, one true and one false.
 */
int test1_5_mutatee() {
  if (test1_5_func1()) {
    return -1; /* Test failed */
  } else {
    test_passes(testname);
    return 0; /* Test passed */
  }
}

void test1_5_func2() {
  dprintf("func5_1 () called\n");
}

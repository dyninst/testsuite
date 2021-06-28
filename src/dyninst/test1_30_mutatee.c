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

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_30_call1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

unsigned long test1_30_globalVariable3 = 0;
unsigned long test1_30_globalVariable4 = 0;
unsigned long test1_30_globalVariable5 = 0;
unsigned long test1_30_globalVariable6 = 0;
/* See also test1_30_globalVariable7, defined below */
unsigned long test1_30_globalVariable8 = 0;
unsigned long test1_30_globalVariable9 = 0;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void func30_2();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* variables to keep the base addr and last addr of test1_30_call1 */
static unsigned long globalVariable30_1 = 0;
static unsigned long globalVariable30_2 = 0;

/* Function definitions follow */


/* this function has to be only 1 line for test30 to pass */
/* these two lines have to be together otherwise test30 will fail */
unsigned long test1_30_globalVariable7 = __LINE__;
void test1_30_call1(){ globalVariable30_1 = __LINE__; globalVariable30_2 = (unsigned long)test1_30_call1;}


int test1_30_mutatee() {
  int passed, retval;

    
    func30_2();

    passed = !test1_30_globalVariable3 ||
	     ((globalVariable30_2 <= test1_30_globalVariable3) &&
	      (test1_30_globalVariable3 <= test1_30_globalVariable9));

    if (!passed){
    	logerror("**Failed** test #30 (line information) in %s[%d]\n", __FILE__, __LINE__ );
	return -1; /* Test failed */
    }

    passed = !test1_30_globalVariable4 ||
	     ((globalVariable30_2 <= test1_30_globalVariable4) &&
	      (test1_30_globalVariable4 <= test1_30_globalVariable9));

    if (!passed) {
    	logerror("**Failed** test #30 (line information) in %s[%d]\n", __FILE__, __LINE__ );
	return -1; /* Test failed */
    }

    passed = !test1_30_globalVariable5 ||
	      ((globalVariable30_2 <= test1_30_globalVariable5) &&
	       (test1_30_globalVariable5 <= test1_30_globalVariable9));

    if (!passed) {
      logerror("**Failed** test #30 (line information) in %s[%d]\n", __FILE__, __LINE__ );
      logerror("gv30_5 = %lu, gv30_2 = %lu, gv30_9 = %lu\n", test1_30_globalVariable5,
	       globalVariable30_2, test1_30_globalVariable9);
      return -1; /* Test failed */
    }

    passed = !test1_30_globalVariable6 ||
	      (globalVariable30_1 == test1_30_globalVariable6);
    if (!passed) {
    	logerror("**Failed** test #30 (line information) in %s[%d]\n", __FILE__, __LINE__ );
	logerror("gv30_6 = %lu, gv30_1 = %lu, should be equal and nonzero!\n", test1_30_globalVariable6,
	       globalVariable30_1);
	return -1; /* Test failed */
    }
	       
    logerror("Passed test #30 (line information)\n");
    retval = 0; /* Test passed */
    test_passes(testname);
    return retval;
}

void func30_2() {
    DUMMY_FN_BODY;
}

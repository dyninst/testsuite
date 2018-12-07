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

// $Id: test1_15.C,v 1.1 2008/10/30 19:19:43 legendre Exp $
/*
 * #Name: test1_15
 * #Desc: Mutator Side - While
 * #Arch: all
 * #Dep: 
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"
#include <limits.h>

#define BA BPatch_arithExpr
#define BC BPatch_constExpr

class test1_15_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_15_factory() 
{
	return new test1_15_Mutator();
}

//
// Start Test Case #5 - mutator side (if w.o. else)
//

test_results_t test1_15_Mutator::executeTest() 
{
	// Find the entry point to the procedure "func5_2"
	const char *funcName = "test1_15_func2";

	BPatch_Vector<BPatch_function *> found_funcs;
	if ((NULL == appImage->findFunction(funcName, found_funcs))
			|| !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n", funcName);
		return FAILED;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *point15_1 = found_funcs[0]->findPoint(BPatch_entry);  

	if (!point15_1 || ((*point15_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return FAILED;
	}

	const char *funcName2 = "test1_15_func1";
	BPatch_Vector<BPatch_function *> found_funcs2;

	if ((NULL == appImage->findFunction(funcName2, found_funcs2))
			|| !found_funcs2.size()) 
	{
		logerror("    Unable to find function %s\n", funcName2);
		return FAILED;
	}

	if (1 < found_funcs2.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs2.size(), funcName2);
	}

	BPatch_Vector<BPatch_point *> *point15_2 = found_funcs2[0]->findPoint(BPatch_subroutine);  

	if (!point15_2 || ((*point15_2).size() == 0)) 
	{
		logerror("Unable to find subroutine call points in \"%s\".\n", funcName2);
		return FAILED;
	}

	BPatch_variableExpr *expr15_1 = findVariable (appImage, "test1_15_globalVariable15_1", point15_1);
	BPatch_variableExpr *expr15_2 = findVariable (appImage, "test1_15_globalVariable15_2", point15_1);
	BPatch_variableExpr *expr15_3 = findVariable (appImage, "test1_15_globalVariable15_3", point15_1);
	BPatch_variableExpr *expr15_4 = findVariable (appImage, "test1_15_globalVariable15_4", point15_1);
	BPatch_variableExpr *expr15_5 = findVariable (appImage, "test1_15_globalVariable15_5", point15_1);


	if (!expr15_1 || !expr15_2 || !expr15_3 || !expr15_4 || !expr15_5)
	{
		logerror("**Failed** test #15 (while)\n");
		logerror("    Unable to locate one of the variables\n");
		return FAILED;
	}

	BPatch_Vector<BPatch_snippet*> vect15_1;

	// Global Variables are initialized as 0s

	// while (expr15_1 < 10) expr15_1++;
	BPatch_whileExpr whileexpr15_1(BPatch_boolExpr(BPatch_lt, *expr15_1,
				BC(10)), 
			BA(BPatch_assign, *expr15_1,
				BA(BPatch_plus, *expr15_1, BC(1))));

	// while (expr15_2 < 1024) expr15_2 >>= 1;
	BPatch_whileExpr whileexpr15_2(BPatch_boolExpr(BPatch_lt, *expr15_2,
				BPatch_constExpr(1024)), 
			BA(BPatch_assign, *expr15_2,
				BA(BPatch_times, *expr15_2, BC(2))));

    // while (expr15_3 > 0 ) expr15_3--
	BPatch_whileExpr whileexpr15_3(BPatch_boolExpr(BPatch_gt, *expr15_3,
				BC(0)),
			BA(BPatch_assign, *expr15_3,
				BA(BPatch_minus, *expr15_3, BC(1))));

    // while (expr15_4 > 1 ) expr15_4/=2
    BPatch_whileExpr whileexpr15_4(BPatch_boolExpr(BPatch_gt, *expr15_4,
                BC(1)),
            BA(BPatch_assign, *expr15_4,
                BA(BPatch_divide, *expr15_4, BC(2))));

    // while (expr15_5 == 1) expr15_5 = 0 (should never happen)
    BPatch_whileExpr whileexpr15_5(BPatch_boolExpr(BPatch_eq, *expr15_5,
                BC(1)),
            BA(BPatch_assign, *expr15_5,
                BC(2)));


	vect15_1.push_back(&whileexpr15_1);
	vect15_1.push_back(&whileexpr15_2);
	vect15_1.push_back(&whileexpr15_3);
	vect15_1.push_back(&whileexpr15_4);
	vect15_1.push_back(&whileexpr15_5);

	BPatch_sequence seexpr15_1(vect15_1);
	checkCost(seexpr15_1);
	if(!appAddrSpace->insertSnippet(seexpr15_1, *point15_1))
        return FAILED;

	return PASSED;
}

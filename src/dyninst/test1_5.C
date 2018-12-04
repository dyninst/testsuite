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

// $Id: test1_5.C,v 1.1 2008/10/30 19:19:43 legendre Exp $
/*
 * #Name: test1_5
 * #Desc: Mutator Side - If without else
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

class test1_5_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_5_factory() 
{
	return new test1_5_Mutator();
}

//
// Start Test Case #5 - mutator side (if w.o. else)
//

test_results_t test1_5_Mutator::executeTest() 
{
	// Find the entry point to the procedure "func5_2"
	const char *funcName = "test1_5_func2";

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

	BPatch_Vector<BPatch_point *> *point5_1 = found_funcs[0]->findPoint(BPatch_entry);  

	if (!point5_1 || ((*point5_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return FAILED;
	}

	const char *funcName2 = "test1_5_func1";
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

	BPatch_Vector<BPatch_point *> *point5_2 = found_funcs2[0]->findPoint(BPatch_subroutine);  

	if (!point5_2 || ((*point5_2).size() == 0)) 
	{
		logerror("Unable to find subroutine call points in \"%s\".\n", funcName2);
		return FAILED;
	}

	BPatch_variableExpr *expr5_1 = findVariable (appImage, "test1_5_globalVariable5_1", point5_1);
	BPatch_variableExpr *expr5_2 = findVariable (appImage, "test1_5_globalVariable5_2", point5_1);
	BPatch_variableExpr *expr5_3 = findVariable (appImage, "test1_5_globalVariable5_3", point5_1);
	BPatch_variableExpr *expr5_4 = findVariable (appImage, "test1_5_globalVariable5_4", point5_1);
	BPatch_variableExpr *expr5_5 = findVariable (appImage, "test1_5_globalVariable5_5", point5_1);
	BPatch_variableExpr *expr5_6 = findVariable (appImage, "test1_5_globalVariable5_6", point5_1);
	BPatch_variableExpr *expr5_7 = findVariable (appImage, "test1_5_globalVariable5_7", point5_1);
	BPatch_variableExpr *expr5_8 = findVariable (appImage, "test1_5_globalVariable5_8", point5_1);
	BPatch_variableExpr *expr5_9 = findVariable (appImage, "test1_5_globalVariable5_9", point5_1);
	BPatch_variableExpr *expr5_10 = findVariable (appImage, "test1_5_globalVariable5_10", point5_1);
	BPatch_variableExpr *expr5_11 = findVariable (appImage, "test1_5_globalVariable5_11", point5_1);
	BPatch_variableExpr *expr5_12 = findVariable (appImage, "test1_5_globalVariable5_12", point5_1);
	BPatch_variableExpr *expr5_13 = findVariable (appImage, "test1_5_globalVariable5_13", point5_1);
	BPatch_variableExpr *expr5_14 = findVariable (appImage, "test1_5_globalVariable5_14", point5_1);
	BPatch_variableExpr *expr5_15 = findVariable (appImage, "test1_5_globalVariable5_15", point5_1);
	BPatch_variableExpr *expr5_16 = findVariable (appImage, "test1_5_globalVariable5_16", point5_1);
	BPatch_variableExpr *expr5_17 = findVariable (appImage, "test1_5_globalVariable5_17", point5_1);


	if (!expr5_1 || !expr5_2 || !expr5_3 || !expr5_4 || !expr5_5 || !expr5_6 || !expr5_7
            || !expr5_8 || !expr5_9 || !expr5_10 || !expr5_11 || !expr5_12 || !expr5_13
            || !expr5_14 || !expr5_15 || !expr5_16 || !expr5_17)
	{
		logerror("**Failed** test #5 (1f w.o. else)\n");
		logerror("    Unable to locate one of the variables\n");
		return FAILED;
	}

	BPatch_Vector<BPatch_snippet*> vect5_1;

	// Global Variables are initialized as 0s

	// if (0 == 1) globalVariable5_1 = 1;
	BPatch_ifExpr ifexpr5_1(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
				BPatch_constExpr(1)), 
			BPatch_arithExpr(BPatch_assign, *expr5_1,
				BPatch_constExpr(1)));

	// if (1 == 1) globalVariable5_2 = 0;
	BPatch_ifExpr ifexpr5_2(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(1),
				BPatch_constExpr(1)), 
			BPatch_arithExpr(BPatch_assign, *expr5_2,
				BPatch_constExpr(0)));

    // if (-1 == -1) globalVariable5_3 = 0
	BPatch_ifExpr ifexpr5_3(BPatch_boolExpr(BPatch_eq, BC(-1),
				BC(-1)),
			BA(BPatch_assign, *expr5_3,
				BC(0)));

    // if (LLONG_MIN == LLONG_MIN) globalVariable5_4 = 0
    BPatch_ifExpr ifexpr5_4(BPatch_boolExpr(BPatch_eq, BC(LLONG_MIN),
                BC(LLONG_MIN)),
            BA(BPatch_assign, *expr5_4,
                BC(0)));

    // if (LLONG_MIN != LLONG_MIN) globalVariable5_5 = 1
    BPatch_ifExpr ifexpr5_5(BPatch_boolExpr(BPatch_ne, BC(LLONG_MIN),
                BC(LLONG_MIN)),
            BA(BPatch_assign, *expr5_5,
                BC(1)));

    // if (LLONG_MAX == LLONG_MAX) globalVariable5_6 = 0
    BPatch_ifExpr ifexpr5_6(BPatch_boolExpr(BPatch_eq, BC(LLONG_MAX),
                BC(LLONG_MAX)),
            BA(BPatch_assign, *expr5_6,
                BC(0)));

    // if (LLONG_MAX == (long long)INT_MAX + 1) globalVariable5_7 = 1
    BPatch_ifExpr ifexpr5_7(BPatch_boolExpr(BPatch_eq, BC(LLONG_MAX),
                BC((long long) INT_MAX + 1)),
            BA(BPatch_assign, *expr5_7,
                BC(1)));

    // if ((long long) -1 == (long long) -1 ) globalVariable5_8 = 0
    BPatch_ifExpr ifexpr5_8(BPatch_boolExpr(BPatch_eq, BC((long long) -1),
                BC((long long) -1)),
            BA(BPatch_assign, *expr5_8,
                BC(0)));

    // LONG_MIN and LONG_MAX tests skipped
    // since the results might be inconsistent on 32-bit
    //
    // if (INT_MIN == INT_MIN) globalVariable5_9 = 0
    BPatch_ifExpr ifexpr5_9(BPatch_boolExpr(BPatch_eq, BC(INT_MIN),
                BC(INT_MIN)),
            BA(BPatch_assign, *expr5_9,
                BC(0)));

    // if (INT_MAX == INT_MAX) globalVariable5_10 = 0
    BPatch_ifExpr ifexpr5_10(BPatch_boolExpr(BPatch_eq, BC(INT_MAX),
                BC(INT_MAX)),
            BA(BPatch_assign, *expr5_10,
                BC(0)));

    // if (false == false) globalVariable5_11 = 0
    BPatch_ifExpr ifexpr5_11(BPatch_boolExpr(BPatch_eq, BC(false),
                BC(false)),
            BA(BPatch_assign, *expr5_11,
                BC(0)));

    // < > <= >=
    // if (long long) INT_MAX < LLONG_MAX, globalVariable5_12 = 0
    BPatch_ifExpr ifexpr5_12(BPatch_boolExpr(BPatch_lt, BC((long long)INT_MAX),
                BC(LLONG_MAX)),
            BA(BPatch_assign, *expr5_12,
                BC(0)));

    // if LLONG_MIN <= LLONG_MAX, globalVariable5_13 = 0
    BPatch_ifExpr ifexpr5_13(BPatch_boolExpr(BPatch_le, BC(LLONG_MIN),
                BC(LLONG_MAX)),
            BA(BPatch_assign, *expr5_13,
                BC(0)));
    
    // if LLONG_MIN <= LLONG_MIN, globalVariable5_14 = 0
    BPatch_ifExpr ifexpr5_14(BPatch_boolExpr(BPatch_le, BC(LLONG_MIN),
                BC(LLONG_MIN)),
            BA(BPatch_assign, *expr5_14,
                BC(0)));

    // if LLONG_MAX >= LLONG_MAX, globalVariable5_15 = 0
    BPatch_ifExpr ifexpr5_15(BPatch_boolExpr(BPatch_ge, BC(LLONG_MAX),
                BC(LLONG_MAX)),
            BA(BPatch_assign, *expr5_15,
                BC(0)));

    // if LLONG_MAX <= LLONG_MIN, globalVariable5_16 = 1
    BPatch_ifExpr ifexpr5_16(BPatch_boolExpr(BPatch_le, BC(LLONG_MAX),
                BC(LLONG_MIN)),
            BA(BPatch_assign, *expr5_16,
                BC(1)));

    // if 0 <= ULLONG_MAX, globalVariable5_17 = 0, unsigned comparsion
    BPatch_ifExpr ifexpr5_17(BPatch_boolExpr(BPatch_le, BC((unsigned long long)0),
                BC(ULLONG_MAX)),
            BA(BPatch_assign, *expr5_17,
                BC(0)));


	vect5_1.push_back(&ifexpr5_1);
	vect5_1.push_back(&ifexpr5_2);
	vect5_1.push_back(&ifexpr5_3);
	vect5_1.push_back(&ifexpr5_4);
	vect5_1.push_back(&ifexpr5_5);
	vect5_1.push_back(&ifexpr5_6);
	vect5_1.push_back(&ifexpr5_7);
	vect5_1.push_back(&ifexpr5_8);
	vect5_1.push_back(&ifexpr5_9);
	vect5_1.push_back(&ifexpr5_10);
	vect5_1.push_back(&ifexpr5_11);
	vect5_1.push_back(&ifexpr5_12);
	vect5_1.push_back(&ifexpr5_13);
	vect5_1.push_back(&ifexpr5_14);
	vect5_1.push_back(&ifexpr5_15);
	vect5_1.push_back(&ifexpr5_16);
	vect5_1.push_back(&ifexpr5_17);

	BPatch_sequence seexpr5_1(vect5_1);
	checkCost(seexpr5_1);
	if(!appAddrSpace->insertSnippet(seexpr5_1, *point5_1))
        return FAILED;

	return PASSED;
}

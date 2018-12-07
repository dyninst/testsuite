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

// $Id: test1_6.C,v 1.1 2008/10/30 19:19:48 legendre Exp $
/*
 * #Name: test1_6
 * #Desc: Mutator Side - Arithmetic Operators
 * #Arch: all
 * #Dep: 
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

class test1_6_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_6_factory() 
{
	return new test1_6_Mutator();
}

//
// Start Test Case #6 - mutator side (arithmetic operators)
//

test_results_t test1_6_Mutator::executeTest() 
{
	// Find the entry point to the procedure "func6_2"
	const char *funcName = "test1_6_func2";
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

	BPatch_Vector<BPatch_point *> *point6_1 = found_funcs[0]->findPoint(BPatch_entry);  

	if (!point6_1 || ((*point6_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return FAILED;
	}

	const char *funcName2 = "test1_6_func2";
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

	BPatch_Vector<BPatch_point *> *point6_2 = found_funcs2[0]->findPoint(BPatch_entry);  

	if (!point6_2 || ((*point6_2).size() == 0)) 
	{
		logerror("Unable to find entry points in \"%s\".\n", funcName2);
		return FAILED;
	}

	BPatch_variableExpr *expr6_1, *expr6_2, *expr6_3, *expr6_4, *expr6_5, *expr6_6, *expr6_7, *expr6_8,
		    *expr6_1a, *expr6_2a, *expr6_3a, *expr6_4a, *expr6_5a, *expr6_6a, *expr6_7a, *expr6_8a,
		    *expr6_1b, *expr6_2b, *expr6_3b, *expr6_4b, *expr6_5b, *expr6_6b, *expr6_7b, *expr6_8b,
            *expr6_9b, *expr6_10b, *expr6_11b, *expr6_12b, *expr6_13b,
		    *constVar1, *constVar2, *constVar3, *constVar5, *constVar6,
		    *constVar10, *constVar60, *constVar64, *constVar66, *constVar67;

	expr6_1 = findVariable(appImage, "test1_6_globalVariable1", point6_2);
	expr6_2 = findVariable(appImage, "test1_6_globalVariable2", point6_2);
	expr6_3 = findVariable(appImage, "test1_6_globalVariable3", point6_2);
	expr6_4 = findVariable(appImage, "test1_6_globalVariable4", point6_2);
	expr6_5 = findVariable(appImage, "test1_6_globalVariable5", point6_2);
	expr6_6 = findVariable(appImage, "test1_6_globalVariable6", point6_2);
	expr6_7 = findVariable(appImage, "test1_6_globalVariable7", point6_2);
	expr6_8 = findVariable(appImage, "test1_6_globalVariable8", point6_2);

	expr6_1a = findVariable(appImage, "test1_6_globalVariable1a", point6_2);
	expr6_2a = findVariable(appImage, "test1_6_globalVariable2a", point6_2);
	expr6_3a = findVariable(appImage, "test1_6_globalVariable3a", point6_2);
	expr6_4a = findVariable(appImage, "test1_6_globalVariable4a", point6_2);
	expr6_5a = findVariable(appImage, "test1_6_globalVariable5a", point6_2);
	expr6_6a = findVariable(appImage, "test1_6_globalVariable6a", point6_2);
	expr6_7a = findVariable(appImage, "test1_6_globalVariable7a", point6_2);
	expr6_8a = findVariable(appImage, "test1_6_globalVariable8a", point6_2);

	expr6_1b = findVariable(appImage, "test1_6_globalVariable1b", point6_2);
	expr6_2b = findVariable(appImage, "test1_6_globalVariable2b", point6_2);
	expr6_3b = findVariable(appImage, "test1_6_globalVariable3b", point6_2);
	expr6_4b = findVariable(appImage, "test1_6_globalVariable4b", point6_2);
	expr6_5b = findVariable(appImage, "test1_6_globalVariable5b", point6_2);
	expr6_6b = findVariable(appImage, "test1_6_globalVariable6b", point6_2);
	expr6_7b = findVariable(appImage, "test1_6_globalVariable7b", point6_2);
	expr6_8b = findVariable(appImage, "test1_6_globalVariable8b", point6_2);
	expr6_9b = findVariable(appImage, "test1_6_globalVariable9b", point6_2);
	expr6_10b = findVariable(appImage, "test1_6_globalVariable10b", point6_2);
	expr6_11b = findVariable(appImage, "test1_6_globalVariable11b", point6_2);
	expr6_12b = findVariable(appImage, "test1_6_globalVariable12b", point6_2);
	expr6_13b = findVariable(appImage, "test1_6_globalVariable13b", point6_2);


	constVar1 = findVariable(appImage, "test1_6_constVar1", point6_2);
	constVar2 = findVariable(appImage, "test1_6_constVar2", point6_2);
	constVar3 = findVariable(appImage, "test1_6_constVar3", point6_2);
	constVar5 = findVariable(appImage, "test1_6_constVar5", point6_2);
	constVar6 = findVariable(appImage, "test1_6_constVar6", point6_2);
	constVar10 = findVariable(appImage, "test1_6_constVar10", point6_2);
	constVar60 = findVariable(appImage, "test1_6_constVar60", point6_2);
	constVar64 = findVariable(appImage, "test1_6_constVar64", point6_2);
	constVar66 = findVariable(appImage, "test1_6_constVar66", point6_2);
	constVar67 = findVariable(appImage, "test1_6_constVar67", point6_2);

	if (!expr6_1 || !expr6_2 || !expr6_3 || !expr6_4 || !expr6_5 ||
			!expr6_6 || !expr6_7 || !expr6_8 || !expr6_1a || !expr6_2a ||
			!expr6_3a || !expr6_4a || !expr6_5a || !expr6_6a ||
			!expr6_7a || !expr6_8a)
	{
		logerror("**Failed** test #6 (arithmetic operators)\n");
		logerror("    Unable to locate one of test1_6_globalVariable?\n");
		return FAILED;
	}

	if (!constVar1 || !constVar2 || !constVar3 || !constVar5 ||
			!constVar6 || !constVar10 || !constVar60 || !constVar64 || 
			!constVar66 || !constVar67) 
	{
		logerror("**Failed** test #6 (arithmetic operators)\n");
		logerror("    Unable to locate one of constVar?\n");
		return FAILED;
	}

	BPatch_Vector<BPatch_snippet*> vect6_1;

	// test1_6_globalVariable1 = 60 + 2
	BPatch_arithExpr arith6_1 (BPatch_assign, *expr6_1,
			BPatch_arithExpr(BPatch_plus,BPatch_constExpr(60), BPatch_constExpr(2)));
	vect6_1.push_back(&arith6_1);

	// test1_6_globalVariable2 = 64 - 1
	BPatch_arithExpr arith6_2 (BPatch_assign, *expr6_2, 
			BPatch_arithExpr(BPatch_minus,BPatch_constExpr(64),BPatch_constExpr(1)));
	vect6_1.push_back(&arith6_2);

	// test1_6_globalVariable3 = 553648128 / 25165824 = 22
	//    - make these big constants to test loading constants larger than
	//      small immediate - jkh 6/22/98

	BPatch_arithExpr arith6_3 (BPatch_assign, *expr6_3, BPatch_arithExpr(
				BPatch_divide,BPatch_constExpr(553648128),BPatch_constExpr(25165824)));
	vect6_1.push_back(&arith6_3);

	// test1_6_globalVariable4 = 67 / 3
	BPatch_arithExpr arith6_4 (BPatch_assign, *expr6_4, BPatch_arithExpr(
				BPatch_divide,BPatch_constExpr(67),BPatch_constExpr(3)));
	vect6_1.push_back(&arith6_4);
	// test1_6_globalVariable5 = 6 * 5
	BPatch_arithExpr arith6_5 (BPatch_assign, *expr6_5, BPatch_arithExpr(
				BPatch_times,BPatch_constExpr(6),BPatch_constExpr(5)));
	vect6_1.push_back(&arith6_5);

	// test1_6_globalVariable6 = 10,3
	BPatch_arithExpr arith6_6 (BPatch_assign, *expr6_6, 
			BPatch_arithExpr(BPatch_seq,BPatch_constExpr(10),BPatch_constExpr(3)));
	vect6_1.push_back(&arith6_6);

	// test1_6_globalVariable7 = 10 / 2
	BPatch_arithExpr arith6_7 (BPatch_assign, *expr6_7,
			BPatch_arithExpr(BPatch_divide,BPatch_constExpr(10),BPatch_constExpr(2)));
	vect6_1.push_back(&arith6_7);

	// test1_6_globalVariable8 = 5 ^ 9
	BPatch_arithExpr arith6_8 (BPatch_assign, *expr6_8,
			BPatch_arithExpr(BPatch_xor,BPatch_constExpr(5),BPatch_constExpr(9)));
	vect6_1.push_back(&arith6_8);

	// test1_6_globalVariable1a = 60 + 2
	BPatch_arithExpr arith6_1a (BPatch_assign, *expr6_1a, 
			BPatch_arithExpr(BPatch_plus, *constVar60, *constVar2));
	vect6_1.push_back(&arith6_1a);

	// test1_6_globalVariable2a = 64 - 1
	BPatch_arithExpr arith6_2a (BPatch_assign, *expr6_2a, 
			BPatch_arithExpr(BPatch_minus, *constVar64, *constVar1));
	vect6_1.push_back(&arith6_2a);

	// test1_6_globalVariable3a = 66 / 3
	BPatch_arithExpr arith6_3a (BPatch_assign, *expr6_3a, BPatch_arithExpr(
				BPatch_divide, *constVar66, *constVar3));
	vect6_1.push_back(&arith6_3a);

	// test1_6_globalVariable4a = 67 / 3
	BPatch_arithExpr arith6_4a (BPatch_assign, *expr6_4a, BPatch_arithExpr(
				BPatch_divide, *constVar67, *constVar3));
	vect6_1.push_back(&arith6_4a);

	// test1_6_globalVariable5a = 6 * 5
	BPatch_arithExpr arith6_5a (BPatch_assign, *expr6_5a, BPatch_arithExpr(
				BPatch_times, *constVar6, *constVar5));
	vect6_1.push_back(&arith6_5a);

	// test1_6_globalVariable6a = 10,3
	// BPatch_arithExpr arith6_6a (BPatch_assign, *expr6_6a, *constVar3);
	//	BPatch_arithExpr(BPatch_seq, *constVar10, BPatch_constExpr(3)));
	BPatch_arithExpr arith6_6a (BPatch_assign, *expr6_6a,
			BPatch_arithExpr(BPatch_seq, *constVar10, *constVar3));
	vect6_1.push_back(&arith6_6a);

	// test1_6_globalVariable7a = 10 / 2
	BPatch_arithExpr arith6_7a (BPatch_assign, *expr6_7a, BPatch_arithExpr(
				BPatch_divide, *constVar10, *constVar2));
	vect6_1.push_back(&arith6_7a);

	// test1_6_globalVariable8a = 67 ^ 10
	BPatch_arithExpr arith6_8a (BPatch_assign, *expr6_8a, BPatch_arithExpr(
				BPatch_xor, *constVar67, *constVar10));
	vect6_1.push_back(&arith6_8a);

    // var1b = -1 + (-1)
    BA arith6_1b (BPatch_assign, *expr6_1b, BA(
                BPatch_plus, BC(-1), BC(-1)
                ));
    vect6_1.push_back(&arith6_1b);

    // var2b = -66666 - (-66666)
    BA arith6_2b (BPatch_assign, *expr6_2b, BA(
                BPatch_minus, BC(-66666), BC(-66666)
                ));
    vect6_1.push_back(&arith6_2b);

    // var3b = INT_MAX - INT_MAX
    BA arith6_3b (BPatch_assign, *expr6_3b, BA(
                BPatch_minus, BC(INT_MAX), BC(INT_MAX)
                ));
    vect6_1.push_back(&arith6_3b);

    // var4b = INT_MAX - LLONG_MAX - (-INT_MIN + LLONG_MIN) = (0)
    BA arith6_4b (BPatch_assign, *expr6_4b, BA(
                BPatch_minus, 
                BA(BPatch_minus, BC((long long)INT_MAX), BC(LLONG_MAX)), 
                BA(BPatch_plus, BC(-(long long)INT_MIN), BC(LLONG_MIN))));
    vect6_1.push_back(&arith6_4b);

    // var5b = 3 * INT_MAX
    BA arith6_5b (BPatch_assign, *expr6_5b, BA(
                BPatch_times, BC((long long)3), BC((long long)INT_MAX)
                ));
    vect6_1.push_back(&arith6_5b);

    // var6b = 10 / 3
    BA arith6_6b (BPatch_assign, *expr6_6b, BA(
                BPatch_divide, BC(10), BC(3)
                ));
    vect6_1.push_back(&arith6_6b);

    // var7b = LLONG_MAX / INT_MAX
    BA arith6_7b (BPatch_assign, *expr6_7b, BA(
                BPatch_divide, BC(LLONG_MAX), BC((long long)INT_MAX)
                ));
    vect6_1.push_back(&arith6_7b);

    // var8b = 0x1deadbeaf * 10  test 64-bit integer multiplication
    BA arith6_8b (BPatch_assign, *expr6_8b, BA(
                BPatch_times, BC(4600387192LL), BC(123LL)
                ));
    vect6_1.push_back(&arith6_8b);

    // var9b = ULLONG_MAX / 100, unsigned 64-bit integer division
    BA arith6_9b (BPatch_assign, *expr6_9b, BA(
                BPatch_divide, BC(ULLONG_MAX), BC((unsigned long long)100)
                ));
    vect6_1.push_back(&arith6_9b);

    // var10b = 3689348814741910323 * 5 == ULLONG_MAX test 64-bit unsigned integer multiplication
    BA arith6_10b (BPatch_assign, *expr6_10b, BA(
                BPatch_times, BC(3689348814741910323ULL), BC(5ULL)
                ));
    vect6_1.push_back(&arith6_10b);

    // var11b = -10 / 5 signed 64-bit integer division
    BA arith6_11b (BPatch_assign, *expr6_11b, BA(
                BPatch_divide, BC(-10), BC(5)
                ));
    vect6_1.push_back(&arith6_11b);

    // var12b = -100 / 4 signed right shift 
    BA arith6_12b (BPatch_assign, *expr6_12b, BA(
                BPatch_divide, BC(-100), BC(4)
                ));
    vect6_1.push_back(&arith6_12b);

    // var13b = ULLONG_MAX / 32 unsigned right shift 
    BA arith6_13b (BPatch_assign, *expr6_13b, BA(
                BPatch_divide, BC(ULLONG_MAX), BC(32ULL)
                ));
    vect6_1.push_back(&arith6_13b);



	checkCost(BPatch_sequence(vect6_1));
	if(!appAddrSpace->insertSnippet( BPatch_sequence(vect6_1), *point6_1))
        return FAILED;

	return PASSED;
}

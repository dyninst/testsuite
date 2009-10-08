/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: test1_4.C,v 1.1 2008/10/30 19:19:34 legendre Exp $
/*
 * #Name: test1_4
 * #Desc: Mutator Side (Sequence) Use the BPatch sequence operation to glue expressions together.  The test is constructed to verify the correct execution order.
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_4_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_4_factory() 
{
	return new test1_4_Mutator();
}

//
// Start Test Case #4 - mutator side (sequence)
//	Use the BPatch sequence operation to glue to expressions togehter.
//	The test is constructed to verify the correct execution order.
//

test_results_t test1_4_Mutator::executeTest() 
{
	// Find the entry point to the procedure "func4_1"
	const char *funcName = "test1_4_func1";
	BPatch_Vector<BPatch_function *> found_funcs;

	if ((NULL == appImage->findFunction(funcName, found_funcs))
			|| !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n",
				funcName);
		return FAILED;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *point4_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!point4_1 || ((*point4_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return FAILED;
	}

	const char *globalVar = "test1_4_globalVariable4_1";
	BPatch_variableExpr *expr4_1 = findVariable (appImage, globalVar, point4_1);

	if (!expr4_1) 
	{
		logerror("**Failed** test #4 (sequence)\n");
		logerror("    Unable to locate variable %s\n", globalVar);
		return FAILED;
	}

	BPatch_arithExpr expr4_2(BPatch_assign, *expr4_1, BPatch_constExpr(42));
	BPatch_arithExpr expr4_3(BPatch_assign, *expr4_1, BPatch_constExpr(43));

	BPatch_Vector<BPatch_snippet*> vect4_1;
	vect4_1.push_back(&expr4_2);
	vect4_1.push_back(&expr4_3);

	BPatch_sequence expr4_4(vect4_1);
	checkCost(expr4_4);
	appAddrSpace->insertSnippet(expr4_4, *point4_1);

	return PASSED;
}

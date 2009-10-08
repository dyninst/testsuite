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

// $Id: test1_32.C,v 1.1 2008/10/30 19:19:02 legendre Exp $
/*
 * #Name: test1_32
 * #Desc: Recursive Base Tramp
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test1_32_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_32_factory() 
{
	return new test1_32_Mutator();
}

//
// Start Test Case #32 - (recursive base tramp)
//

test_results_t test1_32_Mutator::executeTest() 
{
	const char * func32_2_name = "test1_32_func2";
	const char * func32_3_name = "test1_32_func3";
	const char * func32_4_name = "test1_32_func4";

	BPatch_image * app_image = appImage;

	BPatch_Vector<BPatch_function *> bpfv;

	if (NULL == appImage->findFunction(func32_2_name, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", func32_2_name);
		return FAILED;
	}

	BPatch_function *foo_function = bpfv[0];

	bpfv.clear();

	if (NULL == appImage->findFunction(func32_3_name, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", func32_3_name);
		return FAILED;
	}

	BPatch_function *bar_function = bpfv[0];

	bpfv.clear();

	if (NULL == appImage->findFunction(func32_4_name, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", func32_4_name);
		return FAILED;
	}

	BPatch_function *baz_function = bpfv[0];

	bool old_value = BPatch::bpatch->isTrampRecursive();
	BPatch::bpatch->setTrampRecursive( true );

	BPatch_Vector<BPatch_snippet *> foo_args;
	BPatch_snippet * foo_snippet =
		new BPatch_funcCallExpr( * bar_function,
				foo_args );
	instrument_entry_points( appAddrSpace, app_image, foo_function, foo_snippet );

	BPatch_Vector<BPatch_snippet *> bar_args_1;

	bool mutateeFortran = isMutateeFortran(appImage);
	BPatch_constExpr expr32_2;

	if (mutateeFortran) 
	{
		BPatch_process *p = dynamic_cast<BPatch_process *>(appAddrSpace);
		if (!p)
		{
			fprintf(stderr, "%s[%d]:  error:  address space is not process\n", FILE__, __LINE__);
			abort();
		}
		BPatch_variableExpr *expr32_1 = appAddrSpace->malloc (*appImage->findType ("int"));
		expr32_2 = BPatch_constExpr(expr32_1->getBaseAddr ());

		BPatch_arithExpr oneTimeCodeExpr (BPatch_assign, *expr32_1, BPatch_constExpr(1));      
		p->oneTimeCode (oneTimeCodeExpr);
	} 
	else 
	{
		expr32_2 = BPatch_constExpr(1);
	}

	bar_args_1.push_back (&expr32_2);

	BPatch_snippet * bar_snippet_1 =
		new BPatch_funcCallExpr( * baz_function,
				bar_args_1 );
	instrument_entry_points( appAddrSpace, app_image, bar_function, bar_snippet_1 );

	BPatch_Vector<BPatch_snippet *> bar_args_2;

	BPatch_constExpr expr32_5;

	if (mutateeFortran) 
	{
		BPatch_process *p = dynamic_cast<BPatch_process *>(appAddrSpace);
		if (!p)
		{
			fprintf(stderr, "%s[%d]:  error:  address space is not process\n", FILE__, __LINE__);
			abort();
		}
		BPatch_variableExpr *expr32_4 = appAddrSpace->malloc (*appImage->findType ("int"));
		expr32_5 = BPatch_constExpr(expr32_4->getBaseAddr());

		BPatch_arithExpr expr32_6 (BPatch_assign, *expr32_4, BPatch_constExpr (2));
		p->oneTimeCode (expr32_6);

	} 
	else 
	{
		expr32_5 = BPatch_constExpr(2);
	}

	bar_args_2.push_back(&expr32_5);

	BPatch_snippet * bar_snippet_2 =
		new BPatch_funcCallExpr( * baz_function,
				bar_args_2 );
	instrument_exit_points( appAddrSpace, app_image, bar_function, bar_snippet_2 );

	BPatch::bpatch->setTrampRecursive( old_value );

	return PASSED;
}
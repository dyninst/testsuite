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

// $Id: test1_24.C,v 1.1 2008/10/30 19:18:29 legendre Exp $
/*
 * #Name: test1_24
 * #Desc: Mutator Side - Array Variables
 * #Dep: 
 * #Arch: !mips_sgi_irix6_4_test
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_24_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_24_factory() 
{
	return new test1_24_Mutator();
}

//
// Start Test Case #24 - array variables
//

test_results_t test1_24_Mutator::executeTest() 
{
	if (isMutateeFortran(appImage)) 
	{
		return SKIPPED;
	}

	// This test is skipped on some (no?) platforms
#if !defined(sparc_sun_solaris2_4_test) \
	&& !defined(rs6000_ibm_aix4_1_test) \
	&& !defined(alpha_dec_osf4_0_test) \
	&& !defined(i386_unknown_linux2_0_test) \
	&& !defined(x86_64_unknown_linux2_4_test) /* Blind duplication - Ray */ \
	&& !defined(i386_unknown_solaris2_5_test) \
	&& !defined(i386_unknown_nt4_0_test) \
	&& !defined(ia64_unknown_linux2_4_test) \
	&& !defined(os_linux_test) /* Use OS #define instead of platform - Greg */
	return SKIPPED;
#endif

	//     First verify that we can find function call24_1
	BPatch_Vector<BPatch_function *> bpfv;
	char *fn = "test1_24_call1";

	if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
			|| NULL == bpfv[0])
	{
		logerror("    Unable to find function %s\n", fn);
		return FAILED;
	}

	BPatch_function *call24_1_func = bpfv[0];

	BPatch_Vector<BPatch_point *> *temp = call24_1_func->findPoint(BPatch_subroutine);

	//     Then verify that we can find a local variable in call24_1
	if (!temp || (temp->size() == 0)) 
	{
		logerror("**Failed** test #24 (array variables)\n");
		logerror("  can't find function %s\n", fn);
		return FAILED;
	} 
	else 
	{
		dprintf("Found %d callsites in function %s\n", temp->size(), fn);
	}

	BPatch_Vector<BPatch_point *> *point24_1  =
		new (BPatch_Vector<BPatch_point *>);
	point24_1->push_back((*temp)[0]);

	BPatch_Vector<BPatch_point *> *point24_2 = call24_1_func->findPoint(BPatch_exit);
	BPatch_Vector<BPatch_point *> *point24_3 = call24_1_func->findPoint(BPatch_entry);

	if (!point24_1 || !point24_2 || !point24_3) 
	{
		logerror("**Failed** test #24 (array variables)\n");

		if (!point24_1) 
		{
			logerror("    can't find subroutine instrumentation point\n");
		}

		if (!point24_2) 
		{
			logerror("    can't find exit instrumentation point\n");
		}

		if (!point24_3) 
		{
			logerror("    can't find entry instrumentation point\n");
		}
		return FAILED;
	}

	BPatch_variableExpr *lvar;
	BPatch_variableExpr *gvar[10];

	for (int i=1; i <= 9; i++) 
	{
		char name[80];

		sprintf(name, "test1_24_globalVariable%d", i);
		gvar[i] = appImage->findVariable(name);

		if (!gvar[i]) 
		{
			logerror("**Failed** test #24 (array variables)\n");
			logerror("  can't find variable %s\n", name);
			return FAILED;
		}
	}

	lvar = appImage->findVariable(*(*point24_1)[0], "localVariable24_1");

	if (!lvar) 
	{
		logerror("**Failed** test #24 (array variables)\n");
		logerror("  can't find variable localVariable24_1\n");
		return FAILED;
	}

	//     globalVariable24_1[1] = 2400001
	BPatch_arithExpr assignment1(BPatch_assign,
			BPatch_arithExpr(BPatch_ref, *gvar[1], BPatch_constExpr(1)),
			BPatch_constExpr(2400001));
	if (!appAddrSpace->insertSnippet(assignment1, *point24_1))
		return FAILED;

	//     globalVariable24_1[globalVariable24_2] = 2400002
	BPatch_arithExpr assignment2(BPatch_assign,
			BPatch_arithExpr(BPatch_ref, *gvar[1], *gvar[2]),
			BPatch_constExpr(2400002));
	if (!appAddrSpace->insertSnippet(assignment2, *point24_1))
		return FAILED;

	//     globalVariable24_3 = globalVariable24_1[79]
	BPatch_arithExpr assignment3(BPatch_assign, *gvar[3],
			BPatch_arithExpr(BPatch_ref, *gvar[1], BPatch_constExpr(79)));
	if (!appAddrSpace->insertSnippet(assignment3, *point24_1))
		return FAILED;

	//     globalVariable24_5 = globalVariable24_1[globalVariable24_4]
	BPatch_arithExpr assignment4(BPatch_assign, *gvar[5],
			BPatch_arithExpr(BPatch_ref, *gvar[1], *gvar[4]));
	appAddrSpace->insertSnippet(assignment4, *point24_1);

	// now the local variables
	//     localVariable24_1[1] = 2400005
	BPatch_arithExpr *bpae = new BPatch_arithExpr(BPatch_ref, 
			*lvar, 
			BPatch_constExpr(1));
	BPatch_constExpr *bpce = new BPatch_constExpr(2400005);
	BPatch_arithExpr assignment5(BPatch_assign, *bpae, *bpce);
	if (!appAddrSpace->insertSnippet(assignment5, *point24_1))
		return FAILED;

	//     localVariable24_1[globalVariable24_2] = 2400006
	BPatch_arithExpr assignment6(BPatch_assign,
			BPatch_arithExpr(BPatch_ref, *lvar, *gvar[2]),
			BPatch_constExpr(2400006));
	if (!appAddrSpace->insertSnippet(assignment6, *point24_1))
		return FAILED;

	//     globalVariable24_6 = localVariable24_1[79]
	BPatch_arithExpr *ae = 
		new BPatch_arithExpr(BPatch_ref, *lvar, BPatch_constExpr(79));
	BPatch_arithExpr assignment7(BPatch_assign, *gvar[6],*ae);
	if (!appAddrSpace->insertSnippet(assignment7, *point24_1))
		return FAILED;

	//     globalVariable24_7 = localVariable24_1[globalVariable24_4]
	BPatch_arithExpr assignment8(BPatch_assign, *gvar[7],
			BPatch_arithExpr(BPatch_ref, *lvar, *gvar[4]));
	if (!appAddrSpace->insertSnippet(assignment8, *point24_1))
		return FAILED;

	// now test multi-dimensional arrays
	//	   globalVariable24_8[2][3] = 2400011
	BPatch_arithExpr assignment9(BPatch_assign,
			BPatch_arithExpr(BPatch_ref, BPatch_arithExpr(BPatch_ref, *gvar[8],
					BPatch_constExpr(2)), BPatch_constExpr(3)), BPatch_constExpr(2400011));
	if (!appAddrSpace->insertSnippet(assignment9, *point24_1))
		return FAILED;

	// globalVariable24_9 = globalVariable24_8[7][9]
	BPatch_arithExpr assignment10(BPatch_assign, *gvar[9],
			BPatch_arithExpr(BPatch_ref, BPatch_arithExpr(BPatch_ref, *gvar[8],
					BPatch_constExpr(7)), BPatch_constExpr(9)));
	if (!appAddrSpace->insertSnippet(assignment10, *point24_1))
		return FAILED;

	return PASSED;
}
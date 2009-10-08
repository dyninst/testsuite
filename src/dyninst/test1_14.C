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

// $Id: test1_14.C,v 1.1 2008/10/30 19:17:43 legendre Exp $
/*
 * #Name: test1_14
 * #Desc: Mutator Side - Replace Function Call
 * #Dep: 
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"

class test1_14_Mutator : public DyninstMutator {
    const char *libNameAroot;
    char libNameA[128];
public:
    test1_14_Mutator();

    virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test1_14_factory() {
  return new test1_14_Mutator();
}

test1_14_Mutator::test1_14_Mutator() : libNameAroot("libtestA") {}

//
// Start Test Case #14 - mutator side (replace function call)
//
// static int mutatorTest(BPatch_thread *appAddrSpace, BPatch_image *appImage)
// {
test_results_t test1_14_Mutator::executeTest() {
    if ( replaceFunctionCalls(appAddrSpace, appImage, "test1_14_func1",
			      "test1_14_func2", "test1_14_call1", 
			      14, "replace/remove function call", 1) < 0 ) {
        return FAILED;
    }
    if ( replaceFunctionCalls(appAddrSpace, appImage, "test1_14_func1",
			      "test1_14_func3", NULL,
			      14, "replace/remove function call", 1) < 0 ) {
        return FAILED;
    }
    
    int pointer_size = 0;
#if defined(arch_x86_64_test) || defined(ppc64_linux_test)
    pointer_size = pointerSize(appImage);
#endif

    strncpy(libNameA, libNameAroot, 127);
    addLibArchExt(libNameA,127, pointer_size);

    char libA[128];
    snprintf(libA, 128, "./%s", libNameA);
    
    if (!appAddrSpace->loadLibrary(libA)) {
        logerror("**Failed test1_14 (replace function call)\n");
        logerror("  Mutator couldn't load %s into mutatee\n", libNameA);
        return FAILED;
    }

    if ( replaceFunctionCalls(appAddrSpace, appImage, "test1_14_func1",
			      "test1_14_func4", "test1_14_call2_libA",
			      14, "replace/remove function call", 1) < 0 ) {
       return FAILED;
    }

    return PASSED;
} // test1_14_Mutator::executeTest()

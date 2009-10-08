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

// $Id: test_mem_3.C,v 1.1 2008/10/30 19:21:55 legendre Exp $
/*
 * #Name: test6_3
 * #Desc: Prefetch Instrumentation
 * #Dep: 
 * #Arch: !(sparc_sun_solaris2_4_test,,rs6000_ibm_aix4_1_test,i386_unknown_linux2_0_test,x86_64_unknown_linux2_4_test,i386_unknown_nt4_0_test,ia64_unknown_linux2_4_test)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test6.h"

#include "dyninst_comp.h"
class test_mem_3_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test_mem_3_factory() {
  return new test_mem_3_Mutator();
}

#ifdef arch_sparc_test
static const unsigned int nprefes = 2;
static BPatch_memoryAccess* prefeList[nprefes];

static void init_test_data()
{
  int k=-1;
  prefeList[++k] = MK_PF(0,17,0,2);
  prefeList[++k] = MK_PF(0,20,0,0);
}
#endif

#ifdef arch_power_test
static const unsigned int nprefes = 0;
#if defined(__XLC__) || defined(__xlC__)
static BPatch_memoryAccess* *prefeList;
#else
static BPatch_memoryAccess* prefeList[nprefes];
#endif

static void init_test_data()
{
}

#endif

static void *divarwp, *dfvarsp, *dfvardp, *dfvartp, *dlargep;

static void get_vars_addrs(BPatch_image* bip) // from mutatee
{
  BPatch_variableExpr* bpvep_diwarw = bip->findVariable("divarw");
  BPatch_variableExpr* bpvep_diwars = bip->findVariable("dfvars");
  BPatch_variableExpr* bpvep_diward = bip->findVariable("dfvard");
  BPatch_variableExpr* bpvep_diwart = bip->findVariable("dfvart");
  BPatch_variableExpr* bpvep_dlarge = bip->findVariable("dlarge");

  assert(bpvep_diwarw);
  assert(bpvep_diwars);
  assert(bpvep_diward);
  assert(bpvep_diwart);
  assert(bpvep_dlarge);
  
  divarwp = bpvep_diwarw->getBaseAddr();
  dfvarsp = bpvep_diwars->getBaseAddr();
  dfvardp = bpvep_diward->getBaseAddr();
  dfvartp = bpvep_diwart->getBaseAddr();
  dlargep = bpvep_dlarge->getBaseAddr();
}


#if defined(arch_x86_test)
static const unsigned int nprefes = 2;
static BPatch_memoryAccess* prefeList[nprefes + 1]; // for NT

static void init_test_data()
{
  int k=-1;

  prefeList[++k] = MK_PF((long)divarwp,-1,-1,IA32prefetchT0);
  prefeList[++k] = MK_PF((long)divarwp,-1,-1,IA32AMDprefetch);
}
#endif

#ifdef arch_ia64_test
static const unsigned int nprefes = 3;
static BPatch_memoryAccess* prefeList[nprefes];
static void init_test_data() {
	prefeList[0] = MK_PF( 0, 14, -1, 0 );
	prefeList[1] = MK_PF( 0, 14, -1, 0 );
	prefeList[2] = MK_PF( 0, 14, -1, 0 );
}

#endif

#ifdef arch_x86_64_test
static const unsigned int nprefes = 2;

static BPatch_memoryAccess* prefeList[nprefes + 1]; // for NT

static void init_test_data()
{
  int k=-1;

  // prefetches
  prefeList[++k] = MK_PF((long)divarwp,-1,-1,IA32prefetchT0);
  prefeList[++k] = MK_PF((long)divarwp,-1,-1,IA32AMDprefetch);
}
#endif

// Find and instrument loads with a simple function call snippet
// static int mutatorTest(BPatch_thread *bpthr, BPatch_image *bpimg)
test_results_t test_mem_3_Mutator::executeTest() {
  int testnum = 3;
  const char* testdesc = "prefetch instrumentation";
#if !defined(arch_sparc_test) && !defined(arch_power_test) && !defined(arch_x86_test) && !defined(arch_x86_64_test) && !defined(arch_ia64_test)
  //skiptest(testnum, testdesc);
  return SKIPPED;
#else
#if defined(arch_x86_test) || defined(arch_x86_64_test)
  get_vars_addrs(appImage);
#endif
  init_test_data();
  
  BPatch_Set<BPatch_opCode> prefes;
  prefes.insert(BPatch_opPrefetch);

  BPatch_Vector<BPatch_function *> found_funcs;
  const char *inFunction = "loadsnstores";
  if ((NULL == appImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
    logerror("    Unable to find function %s\n",
	    inFunction);
    return FAILED;
  }
       
  if (1 < found_funcs.size()) {
    logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), inFunction);
  }
       
  BPatch_Vector<BPatch_point *> *res1 = found_funcs[0]->findPoint(prefes);

  if(!res1)
    failtest(testnum, testdesc, "Unable to find function \"loadsnstores\".\n");

  dumpvect(res1, "Prefetches");

  if((*res1).size() != nprefes)
    failtest(testnum, testdesc,
             "Number of prefetches seems wrong in function \"loadsnstores.\"\n");

  if(!validate(res1, prefeList, "prefetch"))
    failtest(testnum, testdesc, "Prefetch sequence failed validation.\n");

  if (instCall(appThread, "Prefetch", res1) < 0) {
    return FAILED;
  }

  return PASSED;
#endif
}

// External Interface
// extern "C" TEST_DLL_EXPORT int test6_3_mutatorMAIN(ParameterDict &param)
// {
//     BPatch *bpatch;
//     bpatch = (BPatch *)(param["bpatch"]->getPtr());
//     BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());

//     // Get log file pointers
//     FILE *outlog = (FILE *)(param["outlog"]->getPtr());
//     FILE *errlog = (FILE *)(param["errlog"]->getPtr());
//     setOutputLog(outlog);
//     setErrorLog(errlog);

//     // Read the program's image and get an associated image object
//     BPatch_image *appImage = appThread->getImage();

//     // Run mutator code
//     return mutatorTest(appThread, appImage);
// }
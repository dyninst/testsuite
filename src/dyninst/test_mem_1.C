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

// $Id: test_mem_1.C,v 1.1 2008/10/30 19:21:51 legendre Exp $
/*
 * #Name: test6_1
 * #Desc: Load Instrumentation
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
class test_mem_1_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test_mem_1_factory() {
  return new test_mem_1_Mutator();
}

#ifdef arch_sparc_test
static const unsigned int nloads = 15;
static BPatch_memoryAccess* loadList[nloads];
static void init_test_data()
{
  int k=-1;

  loadList[++k] = MK_LD(0,17,0,4);
  loadList[++k] = MK_LD(3,1,-1,1);
  loadList[++k] = MK_LD(2,2,-1,2);
  loadList[++k] = MK_LD(0,17,0,8);
  loadList[++k] = MK_LD(0,17,0,4);

  loadList[++k] = MK_LS(3,17,-1,1); // ldstub
  loadList[++k] = MK_LD(3,17,-1,1);

  loadList[++k] = MK_LS(0,17,-1,4); // cas
  loadList[++k] = MK_LS(0,17,-1,8); // casx
  loadList[++k] = MK_LS(0,17,0,4);  // swap

  loadList[++k] = MK_LD(0,17,0,4);
  loadList[++k] = MK_LD(0,17,0,4);
  loadList[++k] = MK_LD(0,17,0,8);
  loadList[++k] = MK_LD(0,17,0,8);
  loadList[++k] = MK_LD(0,17,0,16);
}
#endif

#ifdef arch_power_test
static const unsigned int nloads = 41;
static BPatch_memoryAccess* loadList[nloads];
static void init_test_data()
{
  int k=-1;

  loadList[++k] = MK_LD(0, 7, -1, 4); // from la, l sequence

  loadList[++k] = MK_LD(17, 7, -1, 1);
  loadList[++k] = MK_LD(3, 7, -1, 1);

  loadList[++k] = MK_LD(0, 3, 8, 1);
  loadList[++k] = MK_LD(0, 3, 9, 1);

  loadList[++k] = MK_LD(0, 3, -1, 2); // l6
  loadList[++k] = MK_LD(4, 3, -1, 2);
  loadList[++k] = MK_LD(2, 3, -1, 2);
  loadList[++k] = MK_LD(0, 3, -1, 2);

  loadList[++k] = MK_LD(0, 7, 9, 2); // l10
  loadList[++k] = MK_LD(0, 7, 8, 2);
  loadList[++k] = MK_LD(0, 7, 9, 2);
  loadList[++k] = MK_LD(0, 7, 8, 2);

  loadList[++k] = MK_LD(0, 7, -1, 4); // l14
  loadList[++k] = MK_LD(4, 7, -1, 4);
  loadList[++k] = MK_LD(0, 3, 9, 4);
  loadList[++k] = MK_LD(0, 3, 9, 4);

  loadList[++k] = MK_LD(4, 3, -1, 4); // l18
  loadList[++k] = MK_LD(0, 7, 0, 4);  // 0 is 0 for rb...
  loadList[++k] = MK_LD(0, 7, 8, 4);

  loadList[++k] = MK_LD(0, 7, -1, 8); // l21
  loadList[++k] = MK_LD(0, 3, -1, 8);
  loadList[++k] = MK_LD(0, 7, 9, 8);
  loadList[++k] = MK_LD(0, 3, 9, 8);

  loadList[++k] = MK_LD(0, 8, 3, 2);  // l25
  loadList[++k] = MK_LD(0, 9, 3, 4);

  loadList[++k] = MK_LD(-76, 1, -1, 76);  // l27
  loadList[++k] = MK_LD(0, 4, -1, 24);
  loadList[++k] = new BPatch_memoryAccess(NULL, 0,
				   true, false,
				   (long)0, 1, 9,
				   (long)0, POWER_XER2531, -1);

  loadList[++k] = MK_LD(0, -1, 3, 4);  // l30, 0 is -1 in ra...
  loadList[++k] = MK_LD(0, -1, 7, 8);  // l31, idem

  loadList[++k] = MK_LD(0, 4, -1, 4); // from la, l sequence

  loadList[++k] = MK_LD(0, 4, -1, 4);
  loadList[++k] = MK_LD(0, 4, 6, 4);
  loadList[++k] = MK_LD(0, 4, -1, 4);
  loadList[++k] = MK_LD(0, 6, 4, 4);

  loadList[++k] = MK_LD(0, 6, -1, 4); // from la, l sequence

  loadList[++k] = MK_LD(0, 6, -1, 8);
  loadList[++k] = MK_LD(0, 6, 9, 8);
  loadList[++k] = MK_LD(8, 6, -1, 8);
  loadList[++k] = MK_LD(0, 9, 7, 8);
}
#endif

#if defined(i386_unknown_linux2_0_test) \
 || defined(i386_unknown_nt4_0_test)
static const unsigned int nloads = 65;
static BPatch_memoryAccess* loadList[nloads];

static void *divarwp, *dfvarsp, *dfvardp, *dfvartp, *dlargep;

static void get_vars_addrs(BPatch_image* bip) // from mutatee
{
  BPatch_variableExpr* bpvep_diwarw = bip->findVariable("divarw");
  BPatch_variableExpr* bpvep_diwars = bip->findVariable("dfvars");
  BPatch_variableExpr* bpvep_diward = bip->findVariable("dfvard");
  BPatch_variableExpr* bpvep_diwart = bip->findVariable("dfvart");
  BPatch_variableExpr* bpvep_dlarge = bip->findVariable("dlarge");
  
  divarwp = bpvep_diwarw->getBaseAddr();
  dfvarsp = bpvep_diwars->getBaseAddr();
  dfvardp = bpvep_diward->getBaseAddr();
  dfvartp = bpvep_diwart->getBaseAddr();
  dlargep = bpvep_dlarge->getBaseAddr();
}

static void init_test_data()
{
  int k=-1;

  loadList[++k] = MK_LD(0,0,-1,4);
  loadList[++k] = MK_LD(0,1,-1,4);
  loadList[++k] = MK_LD(0,2,-1,4);
  loadList[++k] = MK_LD(0,3,-1,4);
  loadList[++k] = MK_LD((long)divarwp,-1,-1,4);
  loadList[++k] = MK_LD(0,6,-1,4);
  loadList[++k] = MK_LD(0,7,-1,4);

  loadList[++k] = MK_LD(4,0,-1,4); // l8
  loadList[++k] = MK_LD(8,1,-1,4);
  loadList[++k] = MK_LD(4,2,-1,4);
  loadList[++k] = MK_LD(8,3,-1,4);
  loadList[++k] = MK_LD(4,5,-1,4);
  loadList[++k] = MK_LD(8,6,-1,4);
  loadList[++k] = MK_LD(4,7,-1,4);

  loadList[++k] = MK_LD((long)divarwp-1,0,-1,4); // l15
  loadList[++k] = MK_LD((long)divarwp+3,1,-1,4);
  loadList[++k] = MK_LD((long)divarwp+7,2,-1,4);
  loadList[++k] = MK_LD((long)divarwp+11,3,-1,4);
  loadList[++k] = MK_LD((long)divarwp-1,5,-1,4);
  loadList[++k] = MK_LD((long)divarwp+3,6,-1,4);
  loadList[++k] = MK_LD((long)divarwp+7,7,-1,4);

  loadList[++k] = MK_LD(0,3,6,4); // l22
  loadList[++k] = MK_LD(0,4,-1,4);
  loadList[++k] = MK_LDsc(0,3,1,1,4);
  loadList[++k] = MK_LDsc((long)divarwp,-1,1,1,4);
  loadList[++k] = MK_LD(4,3,1,4);
  loadList[++k] = MK_LDsc((long)divarwp,2,2,3,4);
  loadList[++k] = MK_LDsc(2,5,1,1,4); // l28
  loadList[++k] = MK_LDsc(4,3,1,2,4);
  loadList[++k] = MK_LDsc((long)divarwp,5,1,2,4);
  
  loadList[++k] = MK_LD(0,4,-1,4);// l31
  loadList[++k] = MK_LS((long)divarwp+4,-1,-1,4);
  loadList[++k] = MK_LD((long)divarwp+4,-1,-1,4);
  loadList[++k] = MK_LD2(0,6,-1,1,0,7,-1,1);

  loadList[++k] = MK_LS((long)divarwp,-1,-1,4); // l35
  loadList[++k] = MK_LS((long)divarwp+4,-1,-1,4);
  loadList[++k] = MK_LD((long)divarwp+8,-1,-1,4);
//   loadList[++k] = MK_LD((long)divarwp+2,-1,-1,6); // l38
  loadList[++k] = MK_LD((long)divarwp,-1,-1,1);
  loadList[++k] = MK_LS((long)divarwp+4,-1,-1,4); // l40
  loadList[++k] = MK_LD((long)divarwp,-1,-1,4);

  loadList[++k] = MK_LD((long)divarwp,-1,-1,4);
  loadList[++k] = MK_LD((long)divarwp+8,-1,-1,8);

  loadList[++k] = MK_LD((long)dfvarsp,-1,-1,16); // l44
  loadList[++k] = MK_LD((long)dfvarsp,-1,-1,4);

  loadList[++k] = MK_LD((long)dfvardp,-1,-1,16);
  loadList[++k] = MK_LD((long)dfvardp,-1,-1,8);

  loadList[++k] = MK_LD((long)dfvarsp,-1,-1,8); // l48
  loadList[++k] = MK_LD((long)dfvarsp+8,-1,-1,8);
  

  //loadList[++k] = MK_SL2(0,7,-1,4,0,6,-1,4); // l50
  loadList[++k] = MK_SL2vECX(0,7,-1,0,6,-1,2);
  loadList[++k] = new BPatch_memoryAccess(NULL, 0,
					  true, false,
                                          0, 7, -1, 0,
                                          0, -1, IA32_NESCAS, 0, 
                                          -1, false);
  loadList[++k] = new BPatch_memoryAccess(NULL, 0,
					  true, false,
                                          0, 6, -1, 0,
                                          0, -1, IA32_ECMPS, 0,
                                          true, false,
                                          0, 7, -1, 0,
                                          0, -1, IA32_ECMPS, 0);

  loadList[++k] = MK_LD((long)dfvarsp,-1,-1,4);
  loadList[++k] = MK_LD((long)dfvardp,-1,-1,8);
  loadList[++k] = MK_LD((long)dfvartp,-1,-1,10);
  loadList[++k] = MK_LD((long)divarwp,-1,-1,2);
  loadList[++k] = MK_LD((long)divarwp+4,-1,-1,4);
  loadList[++k] = MK_LD((long)divarwp+8,-1,-1,8);

  loadList[++k] = MK_LD((long)divarwp,-1,-1,2);
  loadList[++k] = MK_LD((long)dlargep,-1,-1,28);

  loadList[++k] = MK_LDsccnd((long)divarwp,-1,-1,0,4,7); // cmova
  loadList[++k] = MK_LDsccnd((long)divarwp+4,-1,-1,0,4,4); // cmove
  loadList[++k] = MK_LD((long)divarwp+8,-1,-1,4);

  loadList[++k] = MK_LD(0,4,-1,4); // final pops
  loadList[++k] = MK_LD(0,4,-1,4);
  loadList[++k] = MK_LD(0,4,-1,4);
}
#endif

#ifdef arch_ia64_test
static const unsigned int nloads = 6;
static BPatch_memoryAccess* loadList[nloads];
static void init_test_data() {
	loadList[0] = MK_LD( 0, 16, -1, 8 );
	loadList[1] = MK_LD( 0, 14, -1, 8 );
	loadList[2] = MK_LD( 0, 15, -1, 8 );
	
	loadList[3] = MK_LD( 0, 14, -1, 8 );
	loadList[4] = MK_LD( 0, 14, -1, 16 );
	loadList[5] = MK_LD( 0, 14, -1, 16 );
}
#endif

#ifdef arch_x86_64_test
static const unsigned int nloads = 73;

static BPatch_memoryAccess* loadList[nloads];

static void *divarwp, *dfvarsp, *dfvardp, *dfvartp, *dlargep;

static void get_vars_addrs(BPatch_image* bip) // from mutatee
{

  BPatch_variableExpr* bpvep_diwarw = bip->findVariable("divarw");
  BPatch_variableExpr* bpvep_diwars = bip->findVariable("dfvars");
  BPatch_variableExpr* bpvep_diward = bip->findVariable("dfvard");
  BPatch_variableExpr* bpvep_diwart = bip->findVariable("dfvart");
  BPatch_variableExpr* bpvep_dlarge = bip->findVariable("dlarge");
  divarwp = bpvep_diwarw->getBaseAddr();
  dfvarsp = bpvep_diwars->getBaseAddr();
  dfvardp = bpvep_diward->getBaseAddr();
  dfvartp = bpvep_diwart->getBaseAddr();
  dlargep = bpvep_dlarge->getBaseAddr();
}

static void init_test_data()
{
  int k=-1;

  // ModRM loads

  // mod = 00
  loadList[++k] = MK_LD(0,0,-1,4);
  loadList[++k] = MK_LD(0,1,-1,8);
  loadList[++k] = MK_LD(0,2,-1,4);
  loadList[++k] = MK_LD(0,3,-1,8);
  loadList[++k] = NULL; // rip-relative data access (disable the check)
  loadList[++k] = MK_LD(0,6,-1,8);
  loadList[++k] = MK_LD(0,7,-1,4);
  loadList[++k] = MK_LD(0,8,-1,8);
  loadList[++k] = MK_LD(0,9,-1,4);
  loadList[++k] = MK_LD(0,10,-1,8);
  loadList[++k] = MK_LD(0,11,-1,4);
  loadList[++k] = MK_LD(0,14,-1,8);
  loadList[++k] = MK_LD(0,15,-1,4);

  // mod = 01
  loadList[++k] = MK_LD(4,0,-1,4);
  loadList[++k] = MK_LD(8,1,-1,8);
  loadList[++k] = MK_LD(-4,2,-1,4);
  loadList[++k] = MK_LD(-8,3,-1,8);
  loadList[++k] = MK_LD(4,5,-1,4);
  loadList[++k] = MK_LD(8,6,-1,8);
  loadList[++k] = MK_LD(-4,7,-1,4);
  loadList[++k] = MK_LD(-8,8,-1,8);
  loadList[++k] = MK_LD(4,9,-1,4);
  loadList[++k] = MK_LD(8,10,-1,8);
  loadList[++k] = MK_LD(-4,11,-1,4);
  loadList[++k] = MK_LD(-8,13,-1,8);
  loadList[++k] = MK_LD(127,14,-1,4);
  loadList[++k] = MK_LD(-128,15,-1,8);

  // SIB loads
  loadList[++k] = MK_LD(0,3,6,4);
  loadList[++k] = MK_LD(0,4,-1,8);
  loadList[++k] = MK_LDsc(0,3,1,1,4);
  loadList[++k] = MK_LDsc((long)divarwp,-1,1,1,8);
  loadList[++k] = MK_LD(4,3,1,4);
  loadList[++k] = MK_LDsc((long)divarwp,2,2,3,8);
  loadList[++k] = MK_LDsc(2,5,1,1,4);
  loadList[++k] = MK_LDsc(4,3,1,2,8);
  loadList[++k] = MK_LDsc((long)divarwp,5,1,2,4);

  // loads from semantic test cases
  loadList[++k] = MK_LS((long)divarwp+4,-1,-1,4); // inc
  loadList[++k] = MK_LD((long)divarwp+4,-1,-1,4); // cmp
  loadList[++k] = MK_LD2(0,6,-1,1,0,7,-1,1);      // cmpsb
  loadList[++k] = MK_LS((long)divarwp,-1,-1,4);   // add
  loadList[++k] = MK_LS((long)divarwp+4,-1,-1,4); // xchg
  loadList[++k] = MK_LD((long)divarwp+8,-1,-1,4); // imul
  loadList[++k] = MK_LD((long)divarwp,-1,-1,4);   // imul
  loadList[++k] = MK_LS((long)divarwp+4,-1,-1,4); // shld
  loadList[++k] = MK_LD((long)divarwp,-1,-1,4);   // idiv

  // MMX
  loadList[++k] = MK_LD((long)divarwp,-1,-1,8);
  loadList[++k] = MK_LD((long)divarwp+8,-1,-1,8);

  // SSE
  loadList[++k] = MK_LD((long)dfvartp,-1,-1,16);
  loadList[++k] = MK_LD((long)dfvartp,-1,-1,4);

  // SSE2
  loadList[++k] = MK_LD((long)dfvartp,-1,-1,16);
  loadList[++k] = MK_LD((long)dfvartp,-1,-1,8);

  // 3DNow!
  loadList[++k] = MK_LD((long)dfvardp,-1,-1,8);
  loadList[++k] = MK_LD((long)dfvardp+8,-1,-1,8);

  // REP prefixes
  loadList[++k] = MK_SL2vECX(0,7,-1,0,6,-1,2);
  loadList[++k] = new BPatch_memoryAccess(NULL,0,
					  true, false,
                                          0, 7, -1, 0,
                                          0, -1, IA32_NESCAS, 0, 
                                          -1, false);
  loadList[++k] = new BPatch_memoryAccess(NULL,0,
					  true, false,
                                          0, 6, -1, 0,
                                          0, -1, IA32_ECMPS, 0,
                                          true, false,
                                          0, 7, -1, 0,
                                          0, -1, IA32_ECMPS, 0);

  // x87
  loadList[++k] = MK_LD((long)dfvarsp,-1,-1,4);
  loadList[++k] = MK_LD((long)dfvardp,-1,-1,8);
  loadList[++k] = MK_LD((long)dfvartp,-1,-1,10);
  loadList[++k] = MK_LD((long)divarwp,-1,-1,2);
  loadList[++k] = MK_LD((long)divarwp+4,-1,-1,4);
  loadList[++k] = MK_LD((long)divarwp+8,-1,-1,8);

  loadList[++k] = MK_LD((long)divarwp,-1,-1,2);
  loadList[++k] = MK_LD((long)dlargep,-1,-1,28);

  // conditional moves
  loadList[++k] = MK_LDsccnd((long)divarwp,-1,-1,0,4,7); // cmova
  loadList[++k] = MK_LDsccnd((long)divarwp+4,-1,-1,0,4,4); // cmove
  loadList[++k] = MK_LD((long)divarwp+8,-1,-1,4);

  // final 6 stack pops
  loadList[++k] = MK_LD(0,4,-1,8);
  loadList[++k] = MK_LD(0,4,-1,8);
  loadList[++k] = MK_LD(0,4,-1,8);
  loadList[++k] = MK_LD(0,4,-1,8);
  loadList[++k] = MK_LD(0,4,-1,8);
  loadList[++k] = MK_LD(0,4,-1,8);
}
#endif


// Find and instrument loads with a simple function call snippet
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *bpimg)
test_results_t test_mem_1_Mutator::executeTest() {
  int testnum = 1;
  const char* testdesc = "load instrumentation";
#if !defined(arch_sparc_test) && !defined(arch_power_test) && !defined(arch_x86_test) && !defined(arch_x86_64_test) && !defined(arch_ia64_test)
  //skiptest(testnum, testdesc);
  // Unsupported platform
  return SKIPPED;
#else

#if defined(arch_x86_test) || defined(arch_x86_64_test)
  get_vars_addrs(appImage);
#endif

  dprintf("test_mem_1 is running\n");
  init_test_data();
  
  BPatch_Set<BPatch_opCode> loads;
  loads.insert(BPatch_opLoad);

  BPatch_Vector<BPatch_function *> found_funcs;
  const char *inFunction = "loadsnstores";
  if ((NULL == appImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
    logerror("    Unable to find function %s\n", inFunction);
    return FAILED;
  }
       
  if (1 < found_funcs.size()) {
    logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), inFunction);
  }
       
  BPatch_Vector<BPatch_point *> *res1 = found_funcs[0]->findPoint(loads);

  if(!res1)
    failtest(testnum, testdesc, "Unable to find function \"loadsnstores\".\n");

  dumpvect(res1, "Loads");

  if((*res1).size() != nloads)
  {
    logerror("%s[%d]:  FAILURE: expected %d loads, got %d\n", __FILE__, __LINE__, nloads, (*res1).size());
     failtest(testnum, testdesc, "Number of loads seems wrong in function \"loadsnstores.\"\n");   
  }

  if(!validate(res1, loadList, "load"))
    failtest(testnum, testdesc, "Load sequence failed validation.\n");

  if (instCall(appThread, "Load", res1) < 0) {
    return FAILED;
  }
  return PASSED;
#endif
}

// External Interface
// extern "C" TEST_DLL_EXPORT int test6_1_mutatorMAIN(ParameterDict &param)
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
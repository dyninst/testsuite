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

// $Id: test_mem_5.C,v 1.1 2008/10/30 19:21:59 legendre Exp $
/*
 * #Name: test6_5
 * #Desc: Instrumentation w/effective address snippet
 * #Dep: 
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test6.h"

#include "dyninst_comp.h"
class test_mem_5_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test_mem_5_factory() {
  return new test_mem_5_Mutator();
}

// Instrument all accesses with an effective address snippet
// static int mutatorTest(BPatch_thread *bpthr, BPatch_image *bpimg)
test_results_t test_mem_5_Mutator::executeTest() {
  int testnum = 5;
  const char* testdesc = "instrumentation w/effective address snippet";

  std::set<BPatch_opCode> axs;
  axs.insert(BPatch_opLoad);
  axs.insert(BPatch_opStore);
  axs.insert(BPatch_opPrefetch);


  BPatch_Vector<BPatch_function *> found_funcs;
  const char *inFunction = "loadsnstores";
  if ((NULL == appImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
      failtest(testnum, testdesc, "Unable to find function \"loadsnstores\".\n");
  }
       
  if (1 < found_funcs.size()) {
    logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), inFunction);
  }
       
  BPatch_Vector<BPatch_point *> *res1 = found_funcs[0]->findPoint(axs);

  if(!res1)
    failtest(testnum, testdesc, "Unable to find function \"loadsnstores\".\n");

  //logerror("Doing test %d!!!!!!\n", testnum);
  if (instEffAddr(appAddrSpace, "EffAddr", res1, false) < 0) {
      failtest(testnum, testdesc, "Unable to instrument effective addresses\".\n");
  }
  //appThread->detach(false);
  //appThread->continueExecution();

  return PASSED;
}

// External Interface
// extern "C" TEST_DLL_EXPORT int test6_5_mutatorMAIN(ParameterDict &param)
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

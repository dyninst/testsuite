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

// $Id: test_reloc.C, 2017/05/10 ssunny $
/*
 *
 * #Name: test_reloc
 * #Desc: Relocate all functions without instrumentation
 * #Arch: all
 * #Dep:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"
#include "BPatch_object.h"
#include "test_lib.h"
#include "dyninst_comp.h"
#include <string>

class test_reloc_Mutator : public DyninstMutator {
public:
    virtual test_results_t executeTest();
};

// Factory function.
extern "C" DLLEXPORT TestMutator* test_reloc_factory()
{
    return new test_reloc_Mutator();
}

//
// Start Test Case test_reloc
//
test_results_t test_reloc_Mutator::executeTest() {
    const char *testname = "Relocate all functions in binary without any instrumentation";

    //Get all procedures from binary (only binary procuderes, not the ones in linked libraries too)

    BPatch_Vector<BPatch_function *> *all_funcs = appImage->getProcedures();
    if(NULL == all_funcs) {
        logerror("%s[%d]:  No functions found in binary. Treating as test success.\n",
                 __FILE__, __LINE__);
        return PASSED;
    }

    dprintf("Starting to relocate all %d functions...\n", all_funcs->size());
    // For this test, it is important to use insertion set,
    // because it would take forever to relocate thousands of functions
    // individually. Using insertion set makes sure that we relocate 
    // all functions together.
    appAddrSpace->beginInsertionSet();
    for(BPatch_function *f : *all_funcs) {
    	std::string const& name = f->getModule()->getObject()->name();

    	// Don't relocate anything in the runtime. It causes unpredictable behavior
    	if(name.find("libdyninstAPI_RT.so") != std::string::npos) continue;

        dprintf("Relocation function: %s\n", f->getName().c_str());
        f->relocateFunction();
    }

    appAddrSpace->finalizeInsertionSet(false);

    dprintf("Relocated all functions.\n");
    return PASSED;
}

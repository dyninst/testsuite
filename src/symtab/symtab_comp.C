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

#include "symtab_comp.h"
#include "UsageMonitor.h"
#include <stdlib.h>

using namespace Dyninst;
using namespace SymtabAPI;

SymtabComponent::SymtabComponent()
{
}

SymtabComponent::~SymtabComponent()
{
}

test_results_t SymtabComponent::program_setup(ParameterDict &)
{
   return PASSED;
}

test_results_t SymtabComponent::program_teardown(ParameterDict &)
{
   return PASSED;
}

test_results_t SymtabComponent::group_setup(RunGroup *group, ParameterDict &params)
{
	symtab = NULL;
	//mutatee_p.setString(group->mutatee);
	compiler_p.setString(group->compiler);

	if (measure) um_group.start();  // Measure resource usage.

   if (group->mutatee && group->state != SELFSTART)
   {
	   if (NULL == symtab)
	   {
		   bool result = Symtab::openFile(symtab, std::string(group->mutatee));
		   if (!result || !symtab)
			   return FAILED;
	   }
	   symtab_ptr.setPtr(symtab);
   }
   else
   {
      symtab_ptr.setPtr(NULL);
   }

   if (measure) um_group.end();  // Measure resource usage.

   params["Symtab"] = &symtab_ptr;
   params["createmode"]->setInt(group->createmode);
   //params["mutatee"] = &mutatee_p;
   params["compiler"] = &compiler_p;
   return PASSED;
}

test_results_t SymtabComponent::group_teardown(RunGroup *, ParameterDict &)
{
   symtab = NULL;
   return PASSED;
}

test_results_t SymtabComponent::test_setup(TestInfo *, ParameterDict &)
{
   return PASSED;
}

test_results_t SymtabComponent::test_teardown(TestInfo *, ParameterDict &)
{
   return PASSED;
}

test_results_t SymtabMutator::setup(ParameterDict &param)
{
   symtab = (Symtab *) param["Symtab"]->getPtr();
   createmode = (create_mode_t) param["createmode"]->getInt();
   //mutatee = std::string((const char *)param["mutatee"]->getString());
   compiler = std::string((const char *)param["compiler"]->getString());
   return PASSED;
}

SymtabMutator::~SymtabMutator()
{
}

std::string SymtabComponent::getLastErrorMsg()
{
   return std::string("");
}

TEST_DLL_EXPORT ComponentTester *componentTesterFactory()
{
   return new SymtabComponent();
}

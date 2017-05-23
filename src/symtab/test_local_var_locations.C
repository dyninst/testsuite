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

#include <string>

#include "symtab_comp.h"
#include "test_lib.h"

#include "Symtab.h"
#include "Function.h"

using namespace Dyninst;
using namespace SymtabAPI;

class test_local_var_locations_Mutator : public SymtabMutator {
public:
   test_local_var_locations_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_local_var_locations_factory()
{
   return new test_local_var_locations_Mutator();
}


bool isOutOfInverval(std::pair<Offset, Offset> &interval, Address PC)
{
    return PC < interval.first || PC > interval.second;
}


test_results_t test_local_var_locations_Mutator::executeTest()
{
    // Find function
    std::vector<Function *> funcs;
    std::string functionName = "test_local_var_locations_mutatee";
    bool result = symtab->findFunctionsByName(funcs, functionName);
    if(!result || !funcs.size())
    {
        logerror("[%s:%u] - function (%s) not found.\n", FILE__, __LINE__, functionName.c_str());
        fprintf(stderr, "[%s:%u] - funtion (%s) not found.\n", FILE__, __LINE__, functionName.c_str());
        return FAILED;
    }

    // Function address
    auto offset = funcs[0]->getOffset();  
    auto size = funcs[0]->getSize();  
    auto interval = std::make_pair(offset, offset+size);

    // List of local variable to look for
    std::vector< std::string > variablesToLookUp = {"local_var_locations", "secondVariable", "thisIsReference"};
    std::vector<localVar *> localVariables;

    // For each local variable in function
    for(size_t i=0; i<variablesToLookUp.size(); i++)
    {
        localVariables.clear();
        result = funcs[0]->findLocalVariable(localVariables, variablesToLookUp[i]);
        if(!result || !localVariables.size())
        {
            logerror("[%s:%u] - variable (%s) not found.\n", FILE__, __LINE__, variablesToLookUp[i].c_str());
            fprintf(stderr, "[%s:%u] - variable (%s) not found.\n", FILE__, __LINE__, variablesToLookUp[i].c_str());
            return FAILED;
        }

        // Check for each location of local variable if it is outside function interval
        auto variableLocations = localVariables[0]->getLocationLists();
        if(!variableLocations.size())
        {
            logerror("[%s:%u] - No variable location found.\n", FILE__, __LINE__);
            fprintf(stderr, "[%s:%u] - No variable location found.\n", FILE__, __LINE__);
            return FAILED;
        }

        for(size_t i=0; i<variableLocations.size(); i++)
        {
            // Verify lowPC
            if(isOutOfInverval(interval, variableLocations[i].lowPC))
            {
                logerror("[%s:%u] - lowPC out of function interval.\n", FILE__, __LINE__);
                fprintf(stderr, "[%s:%u] - lowPC out of function interval.\n", FILE__, __LINE__);
                return FAILED;
            }

            // Verify hiPC
            if(variableLocations[i].lowPC <= variableLocations[i].hiPC) 
            {
                if(isOutOfInverval(interval, variableLocations[i].hiPC))
                {
                    logerror("[%s:%u] - hiPC out of function interval.\n", FILE__, __LINE__);
                    fprintf(stderr, "[%s:%u] - hiPC out of function interval.\n", FILE__, __LINE__);
                    return FAILED;
                }
            }else{ // in case of bug?
                if(isOutOfInverval(interval, variableLocations[i].lowPC + variableLocations[i].hiPC))
                {
                    logerror("[%s:%u] - hiPC out of function interval.\n", FILE__, __LINE__);
                    fprintf(stderr, "[%s:%u] - hiPC out of function interval.\n", FILE__, __LINE__);
                    return FAILED;
                }
            }
        }
    }

    return PASSED;
}



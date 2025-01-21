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

// $Id: test1_36.C,v 1.1 2008/10/30 19:19:19 legendre Exp $
/*
 * #Name: test1_36
 * #Desc: Callsite Parameter Referencing
 * #Dep:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"
#include "BPatch_thread.h"

#include "dyninst_comp.h"
#include "test_lib.h"

#include <array>
#include <string>

class test1_36_Mutator : public DyninstMutator {

  virtual test_results_t executeTest();

  BPatch_arithExpr *makeTest36paramExpr(BPatch_snippet *expr, int paramId);
  test_results_t direct_call();
  test_results_t indirect_call();
};

extern "C" DLLEXPORT TestMutator *test1_36_factory() { return new test1_36_Mutator(); }

BPatch_arithExpr *test1_36_Mutator::makeTest36paramExpr(BPatch_snippet *expr, int paramId) {
  if (isMutateeFortran(appImage)) {
    // Fortran is call by reference
    BPatch_arithExpr *derefExpr = new BPatch_arithExpr(BPatch_deref, *(new BPatch_paramExpr(paramId)));
    assert(derefExpr);
    return new BPatch_arithExpr(BPatch_assign, *expr, *derefExpr);
  } else {
    return new BPatch_arithExpr(BPatch_assign, *expr, *(new BPatch_paramExpr(paramId)));
  }
}

test_results_t test1_36_Mutator::direct_call() {
  const char *funcName = "test1_36_func1";
  BPatch_Vector<BPatch_function *> found_funcs;

  if ((NULL == appImage->findFunction(funcName, found_funcs)) || !found_funcs.size()) {
    logerror("    Unable to find function %s\n", funcName);
    return FAILED;
  }

  if (1 < found_funcs.size()) {
    logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", __FILE__, __LINE__,
             found_funcs.size(), funcName);
  }

  BPatch_Vector<BPatch_point *> *all_points36_1 = found_funcs[0]->findPoint(BPatch_subroutine);

  if (!all_points36_1 || (all_points36_1->size() < 1)) {
    logerror("Unable to find point %s - subroutines.\n", funcName);
    return FAILED;
  }

  const char *funcName2 = "test1_36_call1";
  BPatch_point *point36_1 = NULL;

  for (unsigned i = 0; i < (*all_points36_1).size(); i++) {
    BPatch_point *cur_point = (*all_points36_1)[i];
    if (cur_point == NULL)
      continue;

    BPatch_function *func = cur_point->getCalledFunction();
    char funcname[100];

    if (!func)
      continue;

    if (func->getName(funcname, 99)) {
      if (strstr(funcname, funcName2))
        point36_1 = cur_point;
    }
  }

  if (point36_1 == NULL) {
    logerror("Unable to find callsite %s\n", funcName2);
    return FAILED;
  }

  BPatch_Vector<BPatch_snippet *> snippet_seq;
  for(int idx=0; idx<10; idx++) {
    auto var_num = std::to_string(idx + 1);
    auto var_name = "test1_36_globalVariable" + var_num;
    auto expr = findVariable(appImage, var_name.c_str(), all_points36_1);
    if(!expr) {
      logerror("**Failed** test #36 (callsite parameter referencing)\n");
      logerror("    Unable to find variable '%s'\n", var_name.c_str());
      return FAILED;
    }
    snippet_seq.push_back(makeTest36paramExpr(expr, idx));
  }

  BPatch_sequence seqExpr(snippet_seq);
  appAddrSpace->insertSnippet(seqExpr, *point36_1);

  return PASSED;
}

test_results_t test1_36_Mutator::indirect_call() {
  char const *funcName = "test1_36_indirect_call";

  BPatch_Vector<BPatch_function *> found_funcs;
  appImage->findFunction(funcName, found_funcs);

  if (found_funcs.size() != 1U) {
    logerror("Found %u copies of '%s'; expected 1\n", found_funcs.size(), funcName);

    std::string msg;
    for (auto *f : found_funcs) {
      msg += f->getName();
      msg += ", ";
    }
    logerror("findFunction('%s') returned: %s\n", funcName, msg.c_str());

    return FAILED;
  }

  auto *callees = found_funcs[0]->findPoint(BPatch_subroutine);

  const std::array<std::string, 5> expected_callees{"malloc", "strncpy", "toupper", "strncpy", "free"};

  if (!callees) {
    logerror("No call sites found in '%s'; expected %u\n", funcName, expected_callees.size());
    return FAILED;
  }

  if (callees->size() != expected_callees.size()) {
    logerror("Found %u callees in '%s'; expected %u\n", callees->size(), funcName, expected_callees.size());
    return FAILED;
  }

  std::vector<BPatch_function *> called_functions;
  for (auto *c : *callees) {
    auto *f = c->getCalledFunction();
    if (!f) {
      logerror("Unable to get called function in '%s' at address %p\n", funcName, c->getAddress());
      return FAILED;
    }
    called_functions.push_back(f);
  }

  std::string const err_msg = [&expected_callees, called_functions]() {
    std::string msg{"Expected "};
    for (auto &e : expected_callees) {
      msg += e;
      msg += ", ";
    };
    msg += "; Found ";
    for (auto *f : called_functions) {
      msg += f->getName();
      msg += ", ";
    }
    return msg;
  }();

  if (debugPrint()) {
    logstatus("%s\n", err_msg.c_str());
  }

  auto ends = std::mismatch(expected_callees.begin(), expected_callees.end(), called_functions.begin(),
                            [](std::string const &lhs, BPatch_function *rhs) {
                              return rhs->getName().find(lhs) != std::string::npos;
                            });
  if (ends.first != expected_callees.end()) {
    logerror("%s\n", err_msg.c_str());
    return FAILED;
  }

  return PASSED;
}

//
// Start Test Case #36 - (callsite parameter referencing)
//

test_results_t test1_36_Mutator::executeTest() {
  const auto res = direct_call();
  if (res != PASSED) {
    return res;
  }

  if (!appAddrSpace->isStaticExecutable()) {
    return indirect_call();
  }

  return PASSED;
}

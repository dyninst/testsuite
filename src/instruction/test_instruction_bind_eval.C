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

#include "Architecture.h"
#include "Expression.h"
#include "Instruction.h"
#include "instruction_comp.h"
#include "InstructionDecoder.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"
#include "Result.h"
#include "test_lib.h"

#include <array>
#include <vector>

using namespace Dyninst;
using namespace InstructionAPI;

class test_instruction_bind_eval_Mutator : public InstructionMutator {
  test_results_t run(Dyninst::Architecture);

public:
  test_instruction_bind_eval_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* test_instruction_bind_eval_factory() {
  return new test_instruction_bind_eval_Mutator();
}

namespace {
  constexpr auto num_tests = 1;

  std::array<const uint8_t, 7> buffer = {
      0xFF, 0x94, 0xC1, 0xEF, 0xBE, 0xAD, 0xDE // call [8*EAX + ECX + 0xDEADBEEF]
  };
}

test_results_t test_instruction_bind_eval_Mutator::executeTest() {
  auto ret_val = PASSED;
  {
    logerror("**** Running x86 ********\n");
    const auto status = this->run(Dyninst::Arch_x86);
    if(status == FAILED) {
      ret_val = FAILED;
    }
  }
  {
    logerror("**** Running x86_64 ********\n");
    const auto status = this->run(Dyninst::Arch_x86_64);
    if(status == FAILED) {
      ret_val = FAILED;
    }
  }
  return ret_val;
}

test_results_t test_instruction_bind_eval_Mutator::run(Dyninst::Architecture arch) {
  InstructionDecoder d(buffer.data(), buffer.size(), arch);

  std::vector<Instruction> decodedInsns;
  decodedInsns.reserve(num_tests);
  for(int idx = 0; idx < num_tests; idx++) {
    Instruction insn = d.decode();
    if(!insn.isValid()) {
      logerror("Failed to decode test %d\n", idx + 1);
      return FAILED;
    }
    decodedInsns.push_back(insn);
  }

  const auto is_64 = (arch == Dyninst::Arch_x86_64);
  RegisterAST ax(is_64 ? x86_64::rax : x86::eax);
  RegisterAST cx(is_64 ? x86_64::rcx : x86::ecx);

  for(auto&& insn : decodedInsns) {
    Expression::Ptr theCFT = insn.getControlFlowTarget();
    if(!theCFT) {
      logerror("FAILED: no CFT found for '%s'\n", insn.format().c_str());
      return FAILED;
    }
    if(verifyCFT(theCFT, false, 0x1000, u32) == FAILED) {
      return FAILED;
    }

    if(!theCFT->bind(&ax, Result(u32, 3))) {
      logerror("FAILED: bind of EAX failed (insn %s)\n", insn.format().c_str());
      return FAILED;
    }
    if(verifyCFT(theCFT, false, 0x1000, u32) == FAILED) {
      return FAILED;
    }
    if(!theCFT->bind(&cx, Result(u32, 5))) {
      logerror("FAILED: bind of ECX failed\n");
      return FAILED;
    }
    if(verifyCFT(theCFT, false, 0x1000, u32) == FAILED) {
      return FAILED;
    }
    vector<Expression::Ptr> tmp;
    theCFT->getChildren(tmp);
    if(tmp.size() != 1) {
      logerror("FAILED: expected dereference with one child, got %d children\n", tmp.size());
      return FAILED;
    }
    Expression::Ptr memRef = tmp[0];
    if(!memRef) {
      logerror("FAILED: memRef was not an expression\n");
      return FAILED;
    }
    using res_t = typename Result_type2type<u32>::type;
    auto expected_value = static_cast<res_t>(0xDEADBEEF + (0x03 * 0x08 + 0x05));
    if(verifyCFT(memRef, true, expected_value, u32) == FAILED) {
      return FAILED;
    }
  }
  return PASSED;
}

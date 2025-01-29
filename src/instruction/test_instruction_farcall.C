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

#include "Instruction.h"
#include "instruction_comp.h"
#include "InstructionDecoder.h"
#include "test_lib.h"

#include <array>

using namespace Dyninst;
using namespace InstructionAPI;

class test_instruction_farcall_Mutator : public InstructionMutator {
public:
  test_instruction_farcall_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* test_instruction_farcall_factory() {
  return new test_instruction_farcall_Mutator();
}

test_results_t test_instruction_farcall_Mutator::executeTest() {
  constexpr auto num_tests = 1;

  // This form is only valid on 32-bit x86
  std::array<const uint8_t, 9> buffer = {
      0x9A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0xFF, 0xFE // CALL 0504030201, with FF/FE as fenceposts
  };

  InstructionDecoder d(buffer.data(), buffer.size(), Dyninst::Arch_x86);

  for(int idx = 0; idx < num_tests; idx++) {
    Instruction insn = d.decode();
    if(!insn.isValid()) {
      logerror("Failed to decode test %d\n", idx + 1);
      return FAILED;
    }
  }
  return PASSED;
}

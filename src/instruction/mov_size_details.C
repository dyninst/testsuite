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
#include <boost/range/combine.hpp>
#include <tuple>
#include <vector>

namespace di = Dyninst::InstructionAPI;

class mov_size_details_Mutator : public InstructionMutator {
public:
  mov_size_details_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* mov_size_details_factory() {
  return new mov_size_details_Mutator();
}

test_results_t mov_size_details_Mutator::executeTest() {
  constexpr auto num_tests = 5;

  // clang-format off
  constexpr std::array<const uint8_t, 16> buffer = {
    0x66, 0x8c, 0xe8,               // mov ax, gs
    0x89, 0xe8,                     // mov eax, ebp
    0xf2, 0x0f, 0x12, 0xc0,         // movddup xmm0, xmm0
    0x05, 0xef, 0xbe, 0xad, 0xde,   // add eax, 0xdeadbeef
    0xd8, 0xd8,                     // fcomp st0, st0
  };
  // clang-format on

  std::vector<di::Instruction> decodedInsns;
  decodedInsns.reserve(num_tests);

  di::InstructionDecoder d(buffer.data(), buffer.size(), Dyninst::Arch_x86);
  for(int idx = 0; idx < num_tests; idx++) {
    di::Instruction insn = d.decode();
    if(!insn.isValid()) {
      logerror("Failed to decode x86 test %d\n", idx + 1);
      return FAILED;
    }
    decodedInsns.push_back(insn);
  }

  std::vector<std::tuple<int, int>> expected_sizes;

  // mov ax, gs
  expected_sizes.push_back({2, 2});

  // mov eax, ebp
  expected_sizes.push_back({4, 4});

  // movddup xmm0, xmm0
  expected_sizes.push_back({16, 16});

  // add eax, 0xdeadbeef
  expected_sizes.push_back({4, 4});

  // fcomp st0, st0
  expected_sizes.push_back({8, 8}); // wrong. should be 80 bits

  if(decodedInsns.size() != expected_sizes.size()) {
    logerror("FATAL: decodedInsns.size() != expected_sizes.size()");
    return FAILED;
  }

  auto ret_val = PASSED;
  for(auto&& x : boost::combine(decodedInsns, expected_sizes)) {
    const auto insn = x.get<0>();
    const auto sizes = x.get<1>();

    const auto lhs_size = std::get<0>(sizes);
    const auto rhs_size = std::get<1>(sizes);

    di::Expression::Ptr lhs = insn.getOperand(0).getValue();
    di::Expression::Ptr rhs = insn.getOperand(1).getValue();

    if(lhs->size() != lhs_size) {
      logerror("LHS expected %d-bit, actual %d-bit (%s)\n", lhs_size * 8, lhs->size() * 8, lhs->format().c_str());
      ret_val = FAILED;
    }
    if(rhs->size() != rhs_size) {
      logerror("RHS expected %d-bit, actual %d-bit (%s)\n", rhs_size, rhs->size() * 8, rhs->format().c_str());
      ret_val = FAILED;
    }
  }
  return ret_val;
}

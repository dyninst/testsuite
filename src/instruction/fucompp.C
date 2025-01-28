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
#include "Instruction.h"
#include "instruction_comp.h"
#include "InstructionDecoder.h"
#include "registers/x86_regs.h"
#include "test_lib.h"

#include <array>
#include <boost/range/combine.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <vector>

namespace di = Dyninst::InstructionAPI;

class fucompp_Mutator : public InstructionMutator {
public:
  fucompp_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* fucompp_factory() {
  return new fucompp_Mutator();
}

test_results_t fucompp_Mutator::executeTest() {
  constexpr auto num_tests = 24;

  // clang-format off
  constexpr std::array<const uint8_t, 52> buffer = {
      // ordered comparisons
      0xd8, 0x55, 0x00,   // fcom dword ptr [ebp]
      0xdc, 0x55, 0x00,   // fcom qword ptr [ebp]
      0xd8, 0xd0,         // fcom st0
      0xd8, 0xd1,         // fcom st1
      0xd8, 0xd2,         // fcom st2
      0xd8, 0x5d, 0x00,   // fcomp dword ptr [ebp]
      0xdc, 0x5d, 0x00,   // fcomp qword ptr [ebp]
      0xd8, 0xd8,         // fcomp st0
      0xd8, 0xd9,         // fcomp st1
      0xd8, 0xda,         // fcomp st2
      0xde, 0xd9,         // fcompp

      // compare and set eflags
      0xdb, 0xf0,         // fcomi st0, st0
      0xdf, 0xf0,         // fcomip st0, st0
      0xdf, 0xf1,         // fcomip st0, st1
      0xdb, 0xe8,         // fucomi st0, st0
      0xdb, 0xe9,         // fucomi st0, st1
      0xdf, 0xe8,         // fucomip st0, st0
      0xdf, 0xe9,         // fucomip st0, st1

      // unordered comparisons
      0xdd, 0xe0,     // fucom st0
      0xdd, 0xe1,     // fucom st1 (aka, 'fucom')
      0xdd, 0xe2,     // fucom st2
      0xdd, 0xe8,     // fucomp
      0xdd, 0xe9,     // fucompp
      0xdd, 0xea,     // fucomp st2
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

  auto create = [](Dyninst::MachRegister reg) {
    return boost::make_shared<di::RegisterAST>(reg);
  };

  di::RegisterAST::Ptr st0 = create(Dyninst::x86::st0);
  di::RegisterAST::Ptr st1 = create(Dyninst::x86::st1);
  di::RegisterAST::Ptr st2 = create(Dyninst::x86::st2);
  di::RegisterAST::Ptr ebp = create(Dyninst::x86::ebp);

  std::vector<registerSet> expectedRead, expectedWritten;

  // fcom dword ptr [ebp]
  expectedRead.push_back({st0, ebp});
  expectedWritten.push_back({st0});

  // fcom qword ptr [ebp]
  expectedRead.push_back({st0, ebp});
  expectedWritten.push_back({st0});

  // fcom st0
  expectedRead.push_back({st0});
  expectedWritten.push_back({st0});

  // fcom st1
  expectedRead.push_back({st0, st1});
  expectedWritten.push_back({st0});

  // fcom st2
  expectedRead.push_back({st0, st2});
  expectedWritten.push_back({st0});

  // fcomp dword ptr [ebp]
  expectedRead.push_back({st0, ebp});
  expectedWritten.push_back({st0});

  // fcomp qword ptr [ebp]
  expectedRead.push_back({st0, ebp});
  expectedWritten.push_back({st0});

  // fcomp st0
  expectedRead.push_back({st0});
  expectedWritten.push_back({st0});

  // fcomp st1
  expectedRead.push_back({st0, st1});
  expectedWritten.push_back({st0});

  // fcomp st2
  expectedRead.push_back({st0, st2});
  expectedWritten.push_back({st0});

  // fcompp
  expectedRead.push_back({st0, st1});
  expectedWritten.push_back({st0});

  // fcomi st0, st0
  expectedRead.push_back({st0});
  expectedWritten.push_back({st0});

  // fcomip st0, st0
  expectedRead.push_back({st0});
  expectedWritten.push_back({st0});

  // fcomip st0, st1
  expectedRead.push_back({st0, st1});
  expectedWritten.push_back({st0});

  // fucomi st0, st0
  expectedRead.push_back({st0});
  expectedWritten.push_back({st0});

  // fucomi st0, st1
  expectedRead.push_back({st0, st1});
  expectedWritten.push_back({st0});

  // fucomip st0, st0
  expectedRead.push_back({st0});
  expectedWritten.push_back({st0});

  // fucomip st0, st1
  expectedRead.push_back({st0, st1});
  expectedWritten.push_back({st0});

  // fucom st0
  expectedRead.push_back({st0});
  expectedWritten.push_back({});

  // fucom st1 (aka, 'fucom')
  expectedRead.push_back({st0, st1});
  expectedWritten.push_back({});

  // fucom st2
  expectedRead.push_back({st0, st2});
  expectedWritten.push_back({});

  // fucomp
  expectedRead.push_back({st0});
  expectedWritten.push_back({st0});

  // fucompp
  expectedRead.push_back({st0, st1});
  expectedWritten.push_back({st0});

  // fucomp st(3)
  expectedRead.push_back({st0, st2});
  expectedWritten.push_back({st0});

  if(expectedRead.size() != expectedWritten.size()) {
    logerror("FATAL: expectedRead.size() != expectedWritten.size()");
    return FAILED;
  }

  if(expectedRead.size() != decodedInsns.size()) {
    logerror("FATAL: expectedRead.size() != decodedInsns.size()");
    return FAILED;
  }

  test_results_t retVal = PASSED;
  for(auto&& x : boost::combine(decodedInsns, expectedRead, expectedWritten)) {
    auto insn = x.get<0>();
    auto read = x.get<1>();
    auto written = x.get<2>();

    auto status = verify_read_write_sets(insn, read, written);
    if(status == FAILED) {
      retVal = FAILED;
    }
  }
  return retVal;
}

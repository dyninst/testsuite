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
#include "Register.h"
#include "registers/aarch64_regs.h"
#include "test_lib.h"

#include <array>
#include <boost/range/combine.hpp>

using namespace Dyninst;
using namespace InstructionAPI;

class aarch64_cft_Mutator : public InstructionMutator {
public:
  aarch64_cft_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* aarch64_cft_factory() {
  return new aarch64_cft_Mutator();
}

static void reverseBuffer(unsigned char* buffer, int bufferSize) {
  int elementCount = bufferSize / 4;

  for(int loop_index = 0; loop_index < elementCount; loop_index++) {
    std::swap(buffer[0], buffer[3]);
    std::swap(buffer[1], buffer[2]);
    buffer += 4;
  }
}

struct cftExpected {
  bool defined;
  unsigned long long int expected;
  bool call;
  bool conditional;
  bool indirect;
  bool fallthrough;
};

test_results_t verifyTargetType(const Instruction::CFT& actual, const cftExpected& expected) {
  if(actual.isCall != expected.call) {
    logerror("FAILED: expected call = %d, actual = %d\n", expected.call, actual.isCall);
    return FAILED;
  }
  if(actual.isIndirect != expected.indirect) {
    logerror("FAILED: expected indirect = %d, actual = %d\n", expected.indirect, actual.isIndirect);
    return FAILED;
  }
  if(actual.isConditional != expected.conditional) {
    logerror("FAILED: expected conditional = %d, actual = %d\n", expected.conditional, actual.isConditional);
    return FAILED;
  }
  if(actual.isFallthrough != expected.fallthrough) {
    logerror("FAILED: expected fallthrough = %d, actual = %d\n", expected.fallthrough, actual.isFallthrough);
    return FAILED;
  }
  return PASSED;
}

test_results_t aarch64_cft_Mutator::executeTest() {

  struct test_insn {
    std::array<uint8_t, 4> bytes;
    std::vector<cftExpected> expected_targets;
  };

  // clang-format off
  auto tests = std::vector<test_insn> {
    {
      // B #-1
      {0x17, 0xFF, 0xFF, 0xFF},
      {
        {true, 0x3FC, false, false, false, false}
      }
    },
    {
      // BR X12
      {0xD6, 0x1F, 0x01, 0x80},
      {
        {true, 0x90, false, false, true, false},
      }
    },
    {
      // RET X12
      {0xD6, 0x5F, 0x01, 0x80},
      {
        {true, 0x90, false, false, true, false},
      }
    },
    {
      // CBZ W15, #-1
      {0x34, 0xFF, 0xFF, 0xEF},
      {
        {true, 0x3FC, false, true, false, false},
        {true, 0x404, false, true, false, true},
      }
    },
    {
      // B.NE #-1
      {0x54, 0xFF, 0xFF, 0xE1},
      {
        {true, 0x3FC, false, true, false, false},
        {true, 0x404, false, true, false, true},
      }
    },
    {
      // TBZ W4, #30, #-1
      {0x36, 0xF7, 0xFF, 0xE4},
      {
        {true, 0x3FC, false, true, false, false},
        {true, 0x404, false, true, false, true},
      }
    },
    {
      // TBNZ X25, #0, #16
      {0xB7, 0x80, 0x02, 0x19},
      {
        {true, 0x440, false, true, false, false},
        {true, 0x404, false, true, false, true},
      }
    },
    {
      // bl PC + 0x14
      {0x94, 0x00, 0x00, 0x05},
      {
        {true, 0x414, true, false, false, false},
        {true, 0x404, false, false, false, true},
      }
    },
    {
      // BLR X12
      {0xD6, 0x3F, 0x01, 0x80},
      {
        {true, 0x90, true, false, true, false},
        {true, 0x404, false, false, false, true},
      }
    }
  };
  // clang-format on

  Expression::Ptr theIP(new RegisterAST(aarch64::pc));
  Expression::Ptr link_reg(new RegisterAST(aarch64::x30));
  Expression::Ptr x_reg(new RegisterAST(aarch64::x12));

  test_results_t retVal = PASSED;
  auto test_id = 0;

  for(auto&& t : tests) {
    test_id++;

    reverseBuffer(t.bytes.data(), t.bytes.size());
    InstructionDecoder d(t.bytes.data(), t.bytes.size(), Dyninst::Arch_aarch64);

    Instruction insn = d.decode();
    if(!insn.isValid()) {
      logerror("Failed to decode test %d\n", test_id);
      retVal = FAILED;
      continue;
    }

    const auto num_cft = std::distance(insn.cft_begin(), insn.cft_end());
    const auto num_expected_cft = t.expected_targets.size();

    if(num_cft != num_expected_cft) {
      logerror("FAILED: Number of targets mismatched for test %d '%s'. Found %u, expected %u\n", test_id,
               insn.format().c_str(), num_cft, num_expected_cft);
      retVal = FAILED;
      continue;
    }

    auto cft_all = boost::make_iterator_range(insn.cft_begin(), insn.cft_end());
    for(auto&& cft : boost::combine(cft_all, t.expected_targets)) {
      Instruction::CFT cft_cur = cft.get<0>();
      auto target = cft_cur.target;

      if(!target) {
        logerror("FAILED: No target for '%s'\n", insn.format().c_str());
        retVal = FAILED;
        continue;
      }

      target->bind(theIP.get(), Result(u64, 0x400));
      target->bind(x_reg.get(), Result(u64, 0x90));

      cftExpected cft_expected = cft.get<1>();

      auto status = verifyCFT(target, cft_expected.defined, cft_expected.expected, u64);
      if(status == FAILED) {
        retVal = FAILED;
      }

      status = verifyTargetType(cft_cur, cft_expected);
      if(status == FAILED) {
        retVal = FAILED;
      }
    }
  }
  return retVal;
}

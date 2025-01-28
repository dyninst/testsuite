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
#include "registers/ppc32_regs.h"
#include "registers/ppc64_regs.h"
#include "test_lib.h"

#include <array>
#include <boost/range/combine.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <tuple>
#include <vector>

using namespace Dyninst;
using namespace InstructionAPI;

class power_cft_Mutator : public InstructionMutator {
  test_results_t run(Dyninst::Architecture);

public:
  power_cft_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* power_cft_factory() {
  return new power_cft_Mutator();
}

struct cftExpected {
  bool defined;
  unsigned int expected;
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

namespace {
  struct test_insn {
    const uint32_t opcode;
    std::vector<cftExpected> expected_targets;
  };

  // clang-format off
  auto tests = std::vector<test_insn> {
    {
      //  b +16
      {0x48000010},
      {
        {true, 0x410, false, false, false, false},
      }
    },
    {
      //  b +32
      {0x42f00020},
      {
        {true, 0x420, false, false, false, false},
      }
    },
    {
      //  bctr
      {0x4ef00420},
      {
        {true, 44, false, false, true, false},
      }
    },
    {
      //  b 32
      {0x42f00022},
      {
        {true, 0x20, false, false, false, false},
      }
    },
    {
      //  b -32
      {0x42f0ffd0},
      {
        {true, 0x3d0, false, false, false, false},
      }
    },
    {
      //  blr
      {0x4ef00020},
      {
        {true, 0x200, false, false, true, false},
      }
    },
    {
      //  bdnzl cr0
      {0x40010101},
      {
        {true, 0x500, true, true, false, false},
        {true, 0x404, false, false, false, true},
      }
    },
    {
      //  bdnz cr0
      {0x40010100},
      {
        {true, 0x500, false, true, false, false},
        {true, 0x404, false, false, false, true},
      }
    },
    {
      //  bctrl
      {0x4ef00421},
      {
        {true, 44, true, false, true, false},
      }
    },
    {
      //  bnslr+
      {0x4ca30020},
      {
        {true, 0x200, false, true, true, false},
        {true, 0x404, false, false, false, true},
      }
    }
  };
  // clang-format on
}

test_results_t power_cft_Mutator::executeTest() {
  auto ret_val = PASSED;
  {
    logerror("**** Running ppc32 ********\n");
    const auto status = this->run(Dyninst::Arch_ppc32);
    if(status == FAILED) {
      ret_val = FAILED;
    }
  }
  {
    logerror("**** Running ppc64 ********\n");
    const auto status = this->run(Dyninst::Arch_ppc64);
    if(status == FAILED) {
      ret_val = FAILED;
    }
  }
  return ret_val;
}

test_results_t power_cft_Mutator::run(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_ppc64);

  RegisterAST::Ptr pc(new RegisterAST(is_64 ? ppc64::pc : ppc32::pc));
  RegisterAST::Ptr ctr(new RegisterAST(is_64 ? ppc64::ctr : ppc32::ctr));
  RegisterAST::Ptr lr(new RegisterAST(is_64 ? ppc64::lr : ppc32::lr));

  test_results_t retVal = PASSED;
  auto test_id = 0;

  for(auto&& t : tests) {
    test_id++;

    InstructionDecoder d(&(t.opcode), 4, arch);
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

      target->bind(pc.get(), Result(u32, 0x400));
      target->bind(ctr.get(), Result(u32, 44));
      target->bind(lr.get(), Result(u32, 0x200));

      cftExpected cft_expected = cft.get<1>();

      auto status = verifyCFT(target, cft_expected.defined, cft_expected.expected, u32);
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

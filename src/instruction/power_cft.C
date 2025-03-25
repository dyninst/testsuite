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
#include "registers/ppc64_regs.h"
#include "test_lib.h"

#include <boost/optional.hpp>
#include <boost/none_t.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/iterator_range.hpp>
#include <vector>

using namespace Dyninst;
using namespace InstructionAPI;

class power_cft_Mutator : public InstructionMutator {
public:
  power_cft_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* power_cft_factory() {
  return new power_cft_Mutator();
}

struct cftExpected {
  bool is_call;
  bool is_conditional;
  bool is_indirect;
  bool is_fallthrough;
};

test_results_t verifyTargetType(const Instruction::CFT& actual, const cftExpected& expected) {
  if(actual.isCall != expected.is_call) {
    logerror("FAILED: expected call = %d, actual = %d\n", expected.is_call, actual.isCall);
    return FAILED;
  }
  if(actual.isIndirect != expected.is_indirect) {
    logerror("FAILED: expected indirect = %d, actual = %d\n", expected.is_indirect, actual.isIndirect);
    return FAILED;
  }
  if(actual.isConditional != expected.is_conditional) {
    logerror("FAILED: expected conditional = %d, actual = %d\n", expected.is_conditional, actual.isConditional);
    return FAILED;
  }
  if(actual.isFallthrough != expected.is_fallthrough) {
    logerror("FAILED: expected fallthrough = %d, actual = %d\n", expected.is_fallthrough, actual.isFallthrough);
    return FAILED;
  }

  return PASSED;
}

namespace {
  struct test_cft {
    uint32_t value;
    cftExpected cft;
  };
  struct test_insn {
    const uint32_t opcode;
    bool is_branch;
    bool is_return;
    boost::optional<test_cft> pc;   // program counter
    boost::optional<test_cft> ctr;  // count register
    boost::optional<test_cft> lr;   // link register
    boost::optional<test_cft> abs;  // absolute value (AA=1 instructions)
    boost::optional<test_cft> ft;   // fallthrough target
  };

  constexpr auto pc_value = uint32_t{0x400};
  constexpr auto ctr_value = uint32_t{44};
  constexpr auto lr_value = uint32_t{0x200};

  auto pc = boost::make_shared<RegisterAST>(ppc64::pc);
  auto ctr = boost::make_shared<RegisterAST>(ppc64::ctr);
  auto lr = boost::make_shared<RegisterAST>(ppc64::lr);

  std::vector<test_insn> make_tests();
}

test_results_t power_cft_Mutator::executeTest() {
  test_results_t retVal = PASSED;
  auto test_id = 0;

  for(auto const& t : make_tests()) {
    test_id++;

    InstructionDecoder d(&(t.opcode), 4, Dyninst::Arch_ppc64);
    Instruction insn = d.decode();

    if(!insn.isValid()) {
      logerror("Failed to decode test %d\n", test_id);
      retVal = FAILED;
      continue;
    }

    logstatus("\ntest %d: '%s'\n", test_id, insn.format().c_str());

    auto all_cfts = boost::make_iterator_range(insn.cft_begin(), insn.cft_end());
    const auto num_cft = std::distance(insn.cft_begin(), insn.cft_end());

    if(!num_cft) {
      logerror("FAILED: No control flow targets found\n");
      retVal = FAILED;
      continue;
    }

    constexpr auto result_val_t = u64;

    auto bind_val = [](Instruction::CFT const& cur_cft, Expression::Ptr target, uint32_t value) {
      cur_cft.target->bind(target.get(), Result(result_val_t, value));
    };

    auto check_cft = [this](Instruction::CFT const& cur_cft, boost::optional<test_cft> test_reg) {
      if(!test_reg.has_value()) {
        logerror("FAILED: test does not use register, but should\n");
        return FAILED;
      }

      constexpr auto defined = true;
      auto status = verifyCFT(cur_cft.target, defined, test_reg->value, result_val_t);
      if(status == FAILED) {
        return FAILED;
      }

      status = verifyTargetType(cur_cft, test_reg->cft);
      if(status == FAILED) {
        return FAILED;
      }

      return PASSED;
    };

    int const num_targets_expected = [&t]() {
      int n=0;
      if(t.pc) n++;
      if(t.ctr) n++;
      if(t.lr) n++;
      if(t.abs) n++;
      if(t.ft) n++;
      return n;
    }();

    int num_targets_seen = 0;
    bool abs_was_used = false;
    for(auto const& cur_cft : all_cfts) {
      logstatus("Target %d: '%s'\n", num_targets_seen+1, cur_cft.target->format(Dyninst::Arch_ppc64).c_str());

      if(t.abs && !abs_was_used) {
        logstatus("Checking absolute address\n");
        num_targets_seen++;
        bind_val(cur_cft, cur_cft.target, t.abs->value);
        if(check_cft(cur_cft, t.abs) == FAILED) {
          retVal = FAILED;
        }
        abs_was_used = true;
      }
      if(cur_cft.target->isUsed(pc)) {
        logstatus("Checking PC\n");
        num_targets_seen++;
        bind_val(cur_cft, pc, pc_value);
        if(check_cft(cur_cft, t.pc) == FAILED) {
          logstatus("  Trying fallthrough...\n");
          if(!t.ft || (check_cft(cur_cft, t.ft) == FAILED)) {
            retVal = FAILED;
          }
        }
      }
      if(cur_cft.target->isUsed(ctr)) {
        logstatus("Checking count register\n");
        num_targets_seen++;
        bind_val(cur_cft, ctr, ctr_value);
        if(check_cft(cur_cft, t.ctr) == FAILED) {
          retVal = FAILED;
        }
      }
      if(cur_cft.target->isUsed(lr)) {
        logstatus("Checking link register\n");
        num_targets_seen++;
        bind_val(cur_cft, lr, lr_value);
        if(check_cft(cur_cft, t.lr) == FAILED) {
          retVal = FAILED;
        }
      }
    }
    if(num_targets_seen != num_targets_expected) {
      logerror("FAILED: instruction has too %s targets; expected %d, found %d\n",
        (num_targets_seen < num_targets_expected) ? "few" : "many",
        num_targets_expected,
        num_targets_seen
      );
      retVal = FAILED;
    }
    if(insn.isBranch() != t.is_branch) {
      logerror("FAILED: instruction should %s be a branch\n",
               (t.is_branch) ? "" : "not");
      retVal = FAILED;
    }
    if(insn.isReturn() != t.is_return) {
      logerror("FAILED: instruction should %s be a return\n",
              (t.is_return) ? "" : "not");
      retVal = FAILED;
    }
  }

  return retVal;
}

namespace {

  constexpr auto is_call = true;
  constexpr auto is_conditional = true;
  constexpr auto is_indirect = true;
  constexpr auto is_fallthrough = true;
  constexpr auto is_branch = true;
  constexpr auto is_return = true;

  // clang-format off
  std::vector<test_insn> make_tests() {
    return {
      /*
       *  --- Branch I-form ---
       */
      { //  b +16
        0x48000010, is_branch, !is_return,
        test_cft{pc_value + 16, {!is_call, !is_conditional, !is_indirect, !is_fallthrough}}
      },
      { //  ba -48
        0x4bffffd2, is_branch, !is_return
      },
      { //  bl 0x100
        0x48000101, !is_branch, !is_return,
        test_cft{pc_value + 0x100, {is_call, !is_conditional, !is_indirect, !is_fallthrough}}
      },
      { //  bla 0x100
        0x48000103, !is_branch, !is_return
      },

      /*
       *  --- Branch Conditional B-form ---
       */
      { //  bc +32
        0x42f00020, is_branch, !is_return,
        test_cft{pc_value + 32, {!is_call, is_conditional, !is_indirect, !is_fallthrough}},
        {},
        {},
        {},
        test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}}
      },
      { //  bca 32
        0x42f00022, is_branch, !is_return,
        {},
        {},
        {},
        test_cft{32, {!is_call, is_conditional, !is_indirect, !is_fallthrough}},
        test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}},
      },
      { //  bcl +32  (subroutine call to relative address)
        0x42f00021, !is_branch, !is_return,
        test_cft{pc_value + 32, {is_call, is_conditional, !is_indirect, !is_fallthrough}},
        {},
        {},
        {},
        test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}},
      },
      { //  bcla 32  (subroutine call to immediate target)
        0x42f00023, !is_branch, !is_return,
        {},
        {},
        {},
        test_cft{pc_value + 32, {is_call, is_conditional, !is_indirect, !is_fallthrough}},
        test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}}
      },

      /*
       *  --- Branch Conditional to Link Register XL-form ---
       *
       *  There are a large number of uses for this instruction.
       */
      { //  bclr with BH=0 (subroutine return)
        0x4e800020, !is_branch, is_return,
        {},
        {},
        test_cft{lr_value, {!is_call, !is_conditional, is_indirect, !is_fallthrough}},
      },
      { // bclrl  (LK=1)
        0x4e800021, is_branch, !is_return,
        {},
        {},
        test_cft{lr_value, {!is_call, is_conditional, is_indirect, !is_fallthrough}},
        {},
        test_cft{pc_value + 4, {!is_call, !is_conditional, is_indirect, is_fallthrough}},
      },
      { // bdnzfl gt, 0x100
        0x40010101, !is_branch, !is_return,
        test_cft{pc_value + 0x100, {is_call, is_conditional, !is_indirect, !is_fallthrough}},
        {},
        {},
        {},
        test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}}
      },
      { //  bdnzf gt, 0x100   (bdnz cr0)
        0x40010100, is_branch, !is_return,
        test_cft{pc_value + 0x100, {!is_call, is_conditional, !is_indirect, !is_fallthrough}},
        {},
        {},
        {},
        test_cft{pc_value + 4, {!is_call, !is_conditional, !is_indirect, is_fallthrough}},
      },
      { //  bnslr
        0x4ca30020, !is_branch, is_return,
        {},
        {},
        test_cft{lr_value, {!is_call, !is_conditional, is_indirect, !is_fallthrough}},
      },

      /*
       *  --- Branch Conditional to Count Register XL-form ---
       */
      { //  bctr  (LK=0)
        0x4e800420, is_branch, !is_return,
        {},
        test_cft{ctr_value, {!is_call, !is_conditional, is_indirect, !is_fallthrough}},
      },
      { //  bctrl  (LK=1)
        0x4e800421, !is_branch, !is_return,
        {},
        test_cft{ctr_value, {is_call, !is_conditional, is_indirect, !is_fallthrough}},
      },
      { //  bcctr 0x17, 4*cr4+lt, 0
        0x4ef00420, is_branch, !is_return,
        {},
        test_cft{ctr_value, {!is_call, !is_conditional, is_indirect, !is_fallthrough}}
      },

      /*
       *  --- Branch Conditional to Branch Target Address Register XL-form ---
       *
       *  Dyninst can't decode these.
       */
//      { // bctar   (LK=0)
//        0x4e800460
//      },
//      { // bctarl  (LK=1)
//        0x4e800461
//      },
    };
  // clang-format on
  }
}

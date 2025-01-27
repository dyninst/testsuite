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
#include <vector>

using namespace Dyninst;
using namespace InstructionAPI;

class power_decode_Mutator : public InstructionMutator {
  test_results_t run(Dyninst::Architecture);

public:
  power_decode_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* power_decode_factory() {
  return new power_decode_Mutator();
}

namespace {
  constexpr auto num_tests = 23;

  // clang-format off
  std::array<uint32_t const, num_tests> buffer = {
    0x7d204215, // add. r9, r0, r8
    0x7d204214, // add r9, r0, r8
    0x7d204614, // addo r9, r0, r8
    0xfc01102a, // fadd fpr0, fpr1, fpr2
    0xfc01102b, // fadd. fpr0, fpr1, fpr2
    0x38200001, // addi r1, 0, 1
    0x38210001, // addi r1, r1, 1
    0xff800800, // fcmpu fpscw7, fpr0, fpr1
    0x7f800800, // fcmpu cr7, r0, r1
    0x7c0aa120, // mtcrf cr0, cr2, cr4, cr6, r0
    0xfd54058e, // mtfsf fpscw0, fpscw2, fpscw4, fpscw6, fpr0
    0x80010000, // lwz r0, 0(r1)
    0x84010000, // lwzu r0, 0(r1)
    0x7c01102e, // lwzx r0, r2(r1)
    0x7c01106e, // lwzux r0, r2(r1)
    0x7801440c, // rlimi r0, r1
    0x00010090, // fpmul fpr0, fpr1
    0x00010092, // fxmul fpr0, fpr1, or qvfxmadds
    0x00010094, // fxcpmul fpr0, fpr1
    0x00010096, // fxcsmul fpr0, fpr1, or qvfxxnpmadds
    0x40010101, // bdnzl cr0, +0x100
    0x40010100, // bdnz cr0, +0x100
    0x7ca74a6e, // lhzux r9, r7, r5
  };
  // clang-format on
}

test_results_t power_decode_Mutator::executeTest() {
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

test_results_t power_decode_Mutator::run(Dyninst::Architecture arch) {
  std::vector<Instruction> decodedInsns;
  decodedInsns.reserve(num_tests);

  {
    InstructionDecoder d(buffer.data(), buffer.size(), arch);
    for(int idx = 0; idx < num_tests; idx++) {
      Instruction insn = d.decode();
      if(!insn.isValid()) {
        logerror("Failed to decode test %d\n", idx + 1);
        return FAILED;
      }
      decodedInsns.push_back(insn);
    }
  }

  const auto is_64 = (arch == Dyninst::Arch_ppc64);

  RegisterAST::Ptr r0(new RegisterAST(is_64 ? ppc64::r0 : ppc32::r0));
  RegisterAST::Ptr r1(new RegisterAST(is_64 ? ppc64::r1 : ppc32::r1));
  RegisterAST::Ptr r2(new RegisterAST(is_64 ? ppc64::r2 : ppc32::r2));
  RegisterAST::Ptr r5(new RegisterAST(is_64 ? ppc64::r5 : ppc32::r5));
  RegisterAST::Ptr r7(new RegisterAST(is_64 ? ppc64::r7 : ppc32::r7));
  RegisterAST::Ptr r8(new RegisterAST(is_64 ? ppc64::r8 : ppc32::r8));
  RegisterAST::Ptr r9(new RegisterAST(is_64 ? ppc64::r9 : ppc32::r9));
  RegisterAST::Ptr cr0(new RegisterAST(is_64 ? ppc64::cr0 : ppc32::cr0));
  RegisterAST::Ptr cr2(new RegisterAST(is_64 ? ppc64::cr2 : ppc32::cr2));
  RegisterAST::Ptr cr4(new RegisterAST(is_64 ? ppc64::cr4 : ppc32::cr4));
  RegisterAST::Ptr cr6(new RegisterAST(is_64 ? ppc64::cr6 : ppc32::cr6));
  RegisterAST::Ptr cr7(new RegisterAST(is_64 ? ppc64::cr7 : ppc32::cr7));
  RegisterAST::Ptr xer(new RegisterAST(is_64 ? ppc64::xer : ppc32::xer));
  RegisterAST::Ptr fpr0(new RegisterAST(is_64 ? ppc64::fpr0 : ppc32::fpr0));
  RegisterAST::Ptr fpr1(new RegisterAST(is_64 ? ppc64::fpr1 : ppc32::fpr1));
  RegisterAST::Ptr fpr2(new RegisterAST(is_64 ? ppc64::fpr2 : ppc32::fpr2));
  RegisterAST::Ptr fsr0(new RegisterAST(is_64 ? ppc64::fsr0 : ppc32::fsr0));
  RegisterAST::Ptr fsr1(new RegisterAST(is_64 ? ppc64::fsr1 : ppc32::fsr1));
  RegisterAST::Ptr fsr2(new RegisterAST(is_64 ? ppc64::fsr2 : ppc32::fsr2));
  RegisterAST::Ptr fpscw(new RegisterAST(is_64 ? ppc64::fpscw : ppc32::fpscw));
  RegisterAST::Ptr fpscw0(new RegisterAST(is_64 ? ppc64::fpscw0 : ppc32::fpscw0));
  RegisterAST::Ptr fpscw2(new RegisterAST(is_64 ? ppc64::fpscw2 : ppc32::fpscw2));
  RegisterAST::Ptr fpscw4(new RegisterAST(is_64 ? ppc64::fpscw4 : ppc32::fpscw4));
  RegisterAST::Ptr fpscw6(new RegisterAST(is_64 ? ppc64::fpscw6 : ppc32::fpscw6));
  RegisterAST::Ptr fpscw7(new RegisterAST(is_64 ? ppc64::fpscw7 : ppc32::fpscw7));
  RegisterAST::Ptr pc(new RegisterAST(is_64 ? ppc64::pc : ppc32::pc));
  RegisterAST::Ptr ctr(new RegisterAST(is_64 ? ppc64::ctr : ppc32::ctr));
  RegisterAST::Ptr lr(new RegisterAST(is_64 ? ppc64::lr : ppc32::lr));

  std::vector<registerSet> expectedRead, expectedWritten;

  //  add. r9, r0, r8
  expectedRead.push_back({r0, r8});
  expectedWritten.push_back({r9, cr0});

  //  add r9, r0, r8
  expectedRead.push_back({r0, r8});
  expectedWritten.push_back({r9});

  //  addo r9, r0, r8
  expectedRead.push_back({r0, r8});
  expectedWritten.push_back({r9, xer});

  //  fadd fpr0, fpr1, fpr2
  expectedRead.push_back({fpr1, fpr2});
  expectedWritten.push_back({fpr0});

  //  fadd. fpr0, fpr1, fpr2
  expectedRead.push_back({fpr1, fpr2});
  expectedWritten.push_back({fpr0, fpscw});

  //  addi r1, 0, 1
  expectedRead.push_back({});
  expectedWritten.push_back({r1});

  //  addi r1, r1, 1
  expectedRead.push_back({r1});
  expectedWritten.push_back({r1});

  //  fcmpu fpscw7, fpr0, fpr1
  expectedRead.push_back({fpr0, fpr1});
  expectedWritten.push_back({fpscw7});

  //  fcmpu cr7, r0, r1
  expectedRead.push_back({r0, r1});
  expectedWritten.push_back({cr7});

  //  mtcrf cr0, cr2, cr4, cr6, r0
  expectedRead.push_back({r0});
  expectedWritten.push_back({cr0, cr2, cr4, cr6});

  //  mtfsf fpscw0, fpscw2, fpscw4, fpscw6, fpr0
  expectedRead.push_back({fpr0});
  expectedWritten.push_back({fpscw0, fpscw2, fpscw4, fpscw6});

  //  lwz r0, 0(r1)
  expectedRead.push_back({r1});
  expectedWritten.push_back({r0});

  //  lwzu r0, 0(r1)
  expectedRead.push_back({r1});
  expectedWritten.push_back({r0, r1});

  //  lwzx r0, r2(r1)
  expectedRead.push_back({r1, r2});
  expectedWritten.push_back({r0});

  //  lwzux r0, r2(r1)
  expectedRead.push_back({r1, r2});
  expectedWritten.push_back({r0, r1});

  //  rlimi r0, r1
  expectedRead.push_back({r0});
  expectedWritten.push_back({r1});

  //  fpmul fpr0, fpr1
  expectedRead.push_back({fpr1, fpr2, fsr1, fsr2});
  expectedWritten.push_back({fpr0, fsr0});

  //  fxmul fpr0, fpr1, or qvfxmadds
  expectedRead.push_back({fpr1, fpr2, fsr1, fsr2});
  expectedWritten.push_back({fpr0, fsr0});

  //  fxcpmul fpr0, fpr1
  expectedRead.push_back({fpr1, fpr2, fsr2});
  expectedWritten.push_back({fpr0, fsr0});

  //  fxcsmul fpr0, fpr1, or qvfxxnpmadds
  expectedRead.push_back({fpr2, fsr1, fsr2});
  expectedWritten.push_back({fpr0, fsr0});

  //  bdnzl cr0, +0x100
  expectedRead.push_back({pc, cr0, ctr});
  expectedWritten.push_back({pc, ctr, lr});

  //  bdnz cr0, +0x100
  expectedRead.push_back({pc, cr0, ctr});
  expectedWritten.push_back({pc, ctr});

  //  lhzux r9, r7, r5
  expectedRead.push_back({r7, r9});
  expectedWritten.push_back({r5, r7});

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

  {
    uint32_t lhzux = 0x7ca74a6e; // lhzux r9, r7, r5
    InstructionDecoder d(&lhzux, 1, arch);
    Instruction insn = d.decode();
    if(!insn.isValid()) {
      logerror("Failed to decode for memory test %d\n");
      return FAILED;
    }
    if(!insn.readsMemory()) {
      logerror("**FAILED**: insn '%s' did not read memory.\n", insn.format().c_str());
      return FAILED;
    }
  }
  return retVal;
}

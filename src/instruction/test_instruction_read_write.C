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
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"
#include "test_lib.h"

#include <array>
#include <boost/range/combine.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <vector>

using namespace Dyninst;
using namespace InstructionAPI;

class test_instruction_read_write_Mutator : public InstructionMutator {
  test_results_t run(Dyninst::Architecture);
  test_results_t run64_only();

public:
  test_instruction_read_write_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* test_instruction_read_write_factory() {
  return new test_instruction_read_write_Mutator();
}

test_results_t test_instruction_read_write_Mutator::executeTest() {
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
    auto status = this->run(Dyninst::Arch_x86_64);
    if(status == FAILED) {
      ret_val = FAILED;
    }
    status = this->run64_only();
    if(status == FAILED) {
      ret_val = FAILED;
    }
  }
  return ret_val;
}

test_results_t test_instruction_read_write_Mutator::run(Dyninst::Architecture arch) {
  constexpr auto num_tests = 11;

  // clang-format off
  constexpr std::array<const uint8_t, 40> buffer = {
      0x05, 0xef, 0xbe, 0xad, 0xde,             // add eax, 0xdeadbeef
      0x50,                                     // push eax
      0x74, 0x10,                               // jz +0x10(8)
      0xe8, 0x20, 0x00, 0x00, 0x00,             // call +0x20(32)
      0xf8,                                     // clc
      0x04, 0x30,                               // add al, 0x30(8)
      0xc7, 0x45, 0xfc, 0x01, 0x00, 0x00, 0x00, // movl 0x01, -0x4(ebp)
      0x88, 0x55, 0xcc,                         // movb dl, -0x34(ebp)
      0xf2, 0x0f, 0x12, 0xc0,                   // movddup xmm0, xmm0
      0x66, 0x0f, 0x7c, 0xc9,                   // haddpd xmm1, xmm1
      0x8d, 0x83, 0x18, 0xff, 0xff, 0xff        // lea -0xe8(%ebx), %eax
  };
  // clang-format on

  std::vector<Instruction> decodedInsns;
  decodedInsns.reserve(num_tests);

  InstructionDecoder d(buffer.data(), buffer.size(), arch);
  for(int idx = 0; idx < num_tests; idx++) {
    Instruction insn = d.decode();
    if(!insn.isValid()) {
      logerror("Failed to decode x86 test %d\n", idx + 1);
      return FAILED;
    }
    decodedInsns.push_back(insn);
  }

  auto create = [](MachRegister reg) {
    return boost::make_shared<RegisterAST>(reg);
  };

  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  RegisterAST::Ptr eax = create(is_64 ? x86_64::eax : x86::eax);
  RegisterAST::Ptr ebx = create(is_64 ? x86_64::rbx : x86::ebx);
  RegisterAST::Ptr af = create(is_64 ? x86_64::af : x86::af);
  RegisterAST::Ptr zf = create(is_64 ? x86_64::zf : x86::zf);
  RegisterAST::Ptr of = create(is_64 ? x86_64::of : x86::of);
  RegisterAST::Ptr pf = create(is_64 ? x86_64::pf : x86::pf);
  RegisterAST::Ptr sf = create(is_64 ? x86_64::sf : x86::sf);
  RegisterAST::Ptr cf = create(is_64 ? x86_64::cf : x86::cf);
  RegisterAST::Ptr al = create(is_64 ? x86_64::al : x86::al);
  RegisterAST::Ptr bp = create(is_64 ? x86_64::rbp : x86::ebp);
  RegisterAST::Ptr dl = create(is_64 ? x86_64::dl : x86::dl);
  RegisterAST::Ptr xmm0 = create(is_64 ? x86_64::xmm0 : x86::xmm0);
  RegisterAST::Ptr xmm1 = create(is_64 ? x86_64::xmm1 : x86::xmm1);

  RegisterAST::Ptr sp = create(MachRegister::getStackPointer(arch));
  RegisterAST::Ptr ip = create(MachRegister::getPC(arch));

  RegisterAST::Ptr rax = create(x86_64::rax);

  std::vector<registerSet> expectedRead, expectedWritten;

  // add eax, 0xdeadbeef
  expectedRead.push_back({eax});
  expectedWritten.push_back({eax, af, zf, of, pf, sf, cf});

  // push eax
  expectedRead.push_back({sp, (is_64 ? rax : eax)});
  expectedWritten.push_back({sp});

  // jz +0x10(8)
  expectedRead.push_back({zf, sf, cf, pf, of, ip});
  expectedWritten.push_back({ip});

  // call +0x20(32)
  expectedRead.push_back({sp, ip});
  expectedWritten.push_back({sp, ip});

  // clc
  expectedRead.push_back({});
  expectedWritten.push_back({cf});

  // add al, 0x30(8)
  expectedRead.push_back({al});
  expectedWritten.push_back({al, zf, cf, sf, of, pf, af});

  // movl 0x01, -0x4(ebp)
  expectedRead.push_back({bp});
  expectedWritten.push_back({});

  // movb dl, -0x34(ebp)
  expectedRead.push_back({bp, dl});
  expectedWritten.push_back({});

  // movddup xmm0, xmm0
  expectedRead.push_back({xmm0});
  expectedWritten.push_back({xmm0});

  // haddpd xmm1, xmm1
  expectedRead.push_back({xmm1});
  expectedWritten.push_back({xmm1});

  // lea -0xe8(%ebx), %eax
  expectedRead.push_back({ebx});
  expectedWritten.push_back({eax});

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

test_results_t test_instruction_read_write_Mutator::run64_only() {
  constexpr auto arch = Dyninst::Arch_x86_64;
  constexpr auto num_tests = 1;

  // clang-format off
  constexpr std::array<const uint8_t, 4> buffer = {
      0x44, 0x89, 0x45, 0xc4 // mov dword ptr [rbp - 0x3c], r8d
  };
  // clang-format on

  std::vector<Instruction> decodedInsns;
  decodedInsns.reserve(num_tests);

  InstructionDecoder d(buffer.data(), buffer.size(), arch);
  for(int idx = 0; idx < num_tests; idx++) {
    Instruction insn = d.decode();
    if(!insn.isValid()) {
      logerror("Failed to decode x86_64 test %d\n", idx + 1);
      return FAILED;
    }
    decodedInsns.push_back(insn);
  }

  RegisterAST::Ptr r8 = boost::make_shared<RegisterAST>(x86_64::r8d);
  RegisterAST::Ptr rbp = boost::make_shared<RegisterAST>(x86_64::rbp);

  std::vector<registerSet> expectedRead, expectedWritten;

  expectedRead.push_back({rbp, r8});
  expectedWritten.push_back({});

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

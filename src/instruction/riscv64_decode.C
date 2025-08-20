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
#include "registers/riscv64_regs.h"
#include "test_lib.h"

#include <array>
#include <boost/range/combine.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <deque>
#include <vector>

using namespace Dyninst;
using namespace InstructionAPI;

class riscv64_simd_Mutator : public InstructionMutator {
public:
  riscv64_simd_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* riscv64_simd_factory() {
  return new riscv64_simd_Mutator();
}

void reverseBuffer(unsigned char* buffer, int bufferSize) {
  int elementCount = bufferSize / 4;

  for(int loop_index = 0; loop_index < elementCount; loop_index++) {
    std::swap(buffer[0], buffer[3]);
    std::swap(buffer[1], buffer[2]);
    buffer += 4;
  }
}

test_results_t riscv64_simd_Mutator::executeTest() {
  constexpr auto num_tests = 131;

  // clang-format off
  std::array<uint8_t, 4*num_tests> buffer = {
    0x37, 0x00, 0x00, 0x00,     // lui x0, 0x0
    0x37, 0xe4, 0xfb, 0xe0,     // lui x8, 0xe0fbe
    0xb7, 0xff, 0xff, 0xff,     // lui x31, 0xfffff
    0x97, 0x12, 0xbd, 0x77,     // auipc x5, 0x77bd1
    0xef, 0x03, 0x00, 0x80,     // jal x7, -0x100000
    0xef, 0x8e, 0xe6, 0xe3,     // jal x29, -0x979c2
    0x6f, 0xf7, 0xff, 0x7f,     // jal x14, 0xffffe
    0xe7, 0x8c, 0x05, 0x80,     // jalr x25, x11, -0x800
    0xe7, 0x00, 0xb5, 0x60,     // jalr x1, x10, 0x60b
    0x67, 0x00, 0xf6, 0x7f,     // jalr x0, x12, 0x7ff
    0x63, 0x00, 0x29, 0x80,     // beq x18, x2, -4096
    0xe3, 0x8f, 0x65, 0x7e,     // beq x11, x6, 0xffe
    0x63, 0x08, 0x40, 0x35,     // beq x0, x20, 0x350
    0x63, 0x10, 0xd2, 0x00,     // bne x4, x13, 0x0
    0x63, 0xc6, 0xb8, 0x06,     // blt x17, x11, 0x6c
    0x63, 0x52, 0x8e, 0xd8,     // bge x28, x8, -0xa7c
    0x63, 0xea, 0x5b, 0xfc,     // bltu x23, x5, -0x82c
    0x63, 0x77, 0xdf, 0x4f,     // bgeu x30, x29, 0x4ee
    0x03, 0x00, 0x0b, 0x80,     // lb x0, -0x800(x22)
    0x83, 0x82, 0xd7, 0x07,     // lb x5, 0x7d(x15)
    0x03, 0x83, 0xf5, 0x7f,     // lb x6, 0x7ff(x11)
    0x83, 0x94, 0x91, 0x41,     // lh x9, 0x419(x3)
    0x03, 0x2d, 0xfc, 0x93,     // lw x26, -0x6c1(x24)
    0x03, 0x45, 0xb2, 0xb5,     // lbu x10, -0x4a5(x4)
    0x83, 0xd1, 0xd3, 0x95,     // lhu x3, -0x6a3(x7)
    0x23, 0x00, 0xf0, 0x80,     // sb x15, -0x800(x0)
    0x23, 0x80, 0x39, 0x00,     // sb x3, 0(x19)
    0xa3, 0x0f, 0x45, 0x7f,     // sb x20, 0x7ff(x10)
    0x23, 0x9c, 0x0f, 0xab,     // sh x16, -0x548(x31)
    0xa3, 0xa5, 0xf1, 0x6b,     // sw x31, 0x6ab(x3)
    0x13, 0x06, 0xa6, 0x6d,     // addi x12, x12, 0x6da
  };
  // clang-format on

  reverseBuffer(buffer.data(), buffer.size());
  InstructionDecoder d(buffer.data(), buffer.size(), Dyninst::Arch_riscv64);

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

  auto create = [](MachRegister reg) {
    return boost::make_shared<RegisterAST>(reg);
  };

  RegisterAST::Ptr x0 = create(riscv64::x0);
  RegisterAST::Ptr x1 = create(riscv64::x1);
  RegisterAST::Ptr x2 = create(riscv64::x2);
  RegisterAST::Ptr x3 = create(riscv64::x3);
  RegisterAST::Ptr x4 = create(riscv64::x4);
  RegisterAST::Ptr x5 = create(riscv64::x5);
  RegisterAST::Ptr x6 = create(riscv64::x6);
  RegisterAST::Ptr x7 = create(riscv64::x7);
  RegisterAST::Ptr x8 = create(riscv64::x8);
  RegisterAST::Ptr x9 = create(riscv64::x9);
  RegisterAST::Ptr x10 = create(riscv64::x10);
  RegisterAST::Ptr x11 = create(riscv64::x11);
  RegisterAST::Ptr x12 = create(riscv64::x12);
  RegisterAST::Ptr x13 = create(riscv64::x13);
  RegisterAST::Ptr x14 = create(riscv64::x14);
  RegisterAST::Ptr x15 = create(riscv64::x15);
  RegisterAST::Ptr x16 = create(riscv64::x16);
  RegisterAST::Ptr x17 = create(riscv64::x17);
  RegisterAST::Ptr x18 = create(riscv64::x18);
  RegisterAST::Ptr x19 = create(riscv64::x19);
  RegisterAST::Ptr x20 = create(riscv64::x20);
  RegisterAST::Ptr x21 = create(riscv64::x21);
  RegisterAST::Ptr x22 = create(riscv64::x22);
  RegisterAST::Ptr x23 = create(riscv64::x23);
  RegisterAST::Ptr x24 = create(riscv64::x24);
  RegisterAST::Ptr x25 = create(riscv64::x25);
  RegisterAST::Ptr x26 = create(riscv64::x26);
  RegisterAST::Ptr x27 = create(riscv64::x27);
  RegisterAST::Ptr x28 = create(riscv64::x28);
  RegisterAST::Ptr x29 = create(riscv64::x29);
  RegisterAST::Ptr x30 = create(riscv64::x30);
  RegisterAST::Ptr x31 = create(riscv64::x31);
  RegisterAST::Ptr pc = create(riscv64::pc);

  RegisterAST::Ptr sp(new RegisterAST(riscv64::sp));

  std::deque<registerSet> expectedRead, expectedWritten;

  // lui x0, 0
  expectedWritten.push_back({x0});

  // lui x8, 0xe0fbe
  expectedWritten.push_back({x8});

  // lui x31, 0xfffff
  expectedWritten.push_back({x31});

  // auipc x5, 0x77bd1
  expectedRead.push_back({pc});
  expectedWritten.push_back({x5});

  // jal x7, -0x100000
  expectedRead.push_back({pc});
  expectedWritten.push_back({x7});
  expectedWritten.push_back({pc});

  // jal x29, -0x979c2
  expectedRead.push_back({pc});
  expectedWritten.push_back({x29});
  expectedWritten.push_back({pc});

  // jal x14, 0xffffe
  expectedRead.push_back({pc});
  expectedWritten.push_back({x14});
  expectedWritten.push_back({pc});

  // jalr x25, x11, -0x800
  expectedRead.push_back({x11});
  expectedWritten.push_back({x25});
  expectedWritten.push_back({pc});

  // jalr x1, x10, 0x60b
  expectedRead.push_back({x10});
  expectedWritten.push_back({x1});
  expectedWritten.push_back({pc});

  // jalr x0, x12, 0x7ff
  expectedRead.push_back({x12});
  expectedWritten.push_back({x0});
  expectedWritten.push_back({pc});

  // beq x18, x2, -0x1000
  expectedRead.push_back({x18});
  expectedRead.push_back({x2});
  expectedWritten.push_back({x0});

  // beq x11, x6, 0xffe
  expectedRead.push_back({x11});
  expectedRead.push_back({x6});
  expectedWritten.push_back({x0});

  // beq x0, x20, 0x350
  expectedRead.push_back({x0});
  expectedRead.push_back({x20});
  expectedWritten.push_back({x0});

  // bne x4, x13, 0
  expectedRead.push_back({x4});
  expectedRead.push_back({x13});
  expectedWritten.push_back({x0});

  // blt x17, x11, 0x6c
  expectedRead.push_back({x17});
  expectedRead.push_back({x11});
  expectedWritten.push_back({pc});

  // bge x28, x8, -0xx17c
  expectedRead.push_back({x28});
  expectedRead.push_back({x8});
  expectedWritten.push_back({pc});

  // bltu x23, x5, -0x82c
  expectedRead.push_back({x23});
  expectedRead.push_back({x5});
  expectedWritten.push_back({pc});

  // bgeu x30, x29, 0x4ee
  expectedRead.push_back({x30});
  expectedRead.push_back({x29});
  expectedWritten.push_back({pc});

  // lb x0, -0x800(x22)
  expectedWritten.push_back({x0});

  // lb x5, 0x7d(x15)
  expectedWritten.push_back({x5});

  // lb x6, 0x7ff(x11)
  expectedWritten.push_back({x6});

  // lh x9, 0x419(x3)
  expectedWritten.push_back({x9});

  // lw x26, -0x6c1(x24)
  expectedWritten.push_back({x26});

  // lbu x10, -0x4x15(x4)
  expectedWritten.push_back({x10});

  // lhu x3, -0x6x13(x7)
  expectedWritten.push_back({x3});

  // sb x15, -0x800(x0)
  expectedRead.push_back({x15});

  // sb x3, 0(x19)
  expectedRead.push_back({x3});

  // sb x20, 0x7ff(x10)
  expectedRead.push_back({x20});

  // sh x16, -0x548(x31)
  expectedRead.push_back({x16});

  // sw x31, 0x6ab(x3)
  expectedRead.push_back({x31});

  // addi x12, x12, 0x6da
  expectedWritten.push_back({x12});
  expectedRead.push_back({x12});

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

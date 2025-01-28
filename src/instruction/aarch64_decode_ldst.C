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
#include <boost/smart_ptr/make_shared.hpp>
#include <vector>

using namespace Dyninst;
using namespace InstructionAPI;

class aarch64_decode_ldst_Mutator : public InstructionMutator {
public:
  aarch64_decode_ldst_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* aarch64_decode_ldst_factory() {
  return new aarch64_decode_ldst_Mutator();
}

static void reverseBuffer(unsigned char* buffer, int bufferSize) {
  int elementCount = bufferSize / 4;

  for(int loop_index = 0; loop_index < elementCount; loop_index++) {
    std::swap(buffer[0], buffer[3]);
    std::swap(buffer[1], buffer[2]);
    buffer += 4;
  }
}

test_results_t aarch64_decode_ldst_Mutator::executeTest() {
  constexpr auto num_tests = 136;

  // clang-format off
  std::array<unsigned char, 4*num_tests> buffer = {
    //literal
    0x58, 0x00, 0x00, 0x21,         // ldr x1,  #4
    0x58, 0x00, 0x08, 0x01,         // ldr x1,  #256
    0x18, 0x00, 0x00, 0x21,         // ldr w1,  #4
    0x18, 0x00, 0x08, 0x01,         // ldr w1,  #1048576

    //post-inc
    0xf8, 0x40, 0x14, 0x41,         // ldr,     x1, [x2], #1
    0xf8, 0x4f, 0xf4, 0x41,         // ldr,     x1, [x2], #255
    0x38, 0x40, 0x14, 0x41,         // ldrb,    w1, [x2], #1
    0x38, 0xc0, 0x14, 0x41,         // ldrsb,   w1, [x2], #1
    0x78, 0x40, 0x14, 0x41,         // ldrh,    w1, [x2], #1
    0x78, 0xc0, 0x14, 0x41,         // ldrsh,    x1, [x2], #1

    //imm
    0xf9, 0x40, 0x04, 0x41,         // ldr,     x1, [x2, #8]
    0xf9, 0x40, 0x80, 0x41,         // ldr,     x1, [x2, #256]
    0x39, 0x40, 0x10, 0x41,         // ldrb,    w1, [x2, #4]
    0x39, 0xc0, 0x10, 0x41,         // ldrsb,   w1, [x2, #4]
    0x79, 0x40, 0x08, 0x41,         // ldrh,    w1, [x2, #4]
    0x79, 0xc0, 0x08, 0x41,         // ldrsh,   w1, [x2, #4]
    0xb9, 0x80, 0x04, 0x41,         // ldrsw,   x1, [x2, #4]

    //register off not ext
    0xf8, 0x63, 0x68, 0x41,         // ldr,     x1, [x2, x3]
    0x38, 0x63, 0x68, 0x41,         // ldrb,    w1, [x2, x3]
    0x38, 0xe3, 0x68, 0x41,         // ldrsb,   w1, [x2, x3]
    0x78, 0x63, 0x68, 0x41,         // ldrh,    w1, [x2, x3]
    0x78, 0xe3, 0x68, 0x41,         // ldrsh,   w1, [x2, x3]
    0xb8, 0xa3, 0x68, 0x41,         // ldrsw,    x1, [x2, x3]

    //pre-inc
    0xf8, 0x40, 0x1c, 0x41,         // ldr,     x1, [x2, #1]!
    0xf8, 0x4f, 0xfc, 0x41,         // ldr,     x1, [x2, #255]!
    0x38, 0x40, 0x1c, 0x41,         // ldrb,    w1, [x2, #1]!
    0x38, 0xc0, 0x1c, 0x41,         // ldrsb,   w1, [x2, #1]!
    0x78, 0x40, 0x1c, 0x41,         // ldrh,    w1, [x2, #1]!
    0x78, 0xc0, 0x1c, 0x41,         // ldrsh,   w1, [x2, #1]!
    0xb8, 0x80, 0x1c, 0x41,         // ldrsw,   x1, [x2, #1]!

    //exclusive
    0xc8, 0xdf, 0xfc, 0x41,          //ldar    x1, [x2]
    0x88, 0xdf, 0xfc, 0x41,          //ldar    w1, [x2]
    0x08, 0x5f, 0x7c, 0x41,          //ldxrb   w1, [x2]
    0x08, 0x5f, 0xfc, 0x41,          //ldaxrb  w1, [x2]

    0x08, 0xdf, 0xfc, 0x41,          //ldarb   w1, [x2]
    0x48, 0x5f, 0x7c, 0x41,          //ldxrh   w1, [x2]
    0x48, 0x5f, 0xfc, 0x41,          //ldaxrh  w1, [x2]
    0x48, 0xdf, 0xfc, 0x41,          //ldarh   w1, [x2]

    0xc8, 0x5f, 0x7c, 0x41,          //ldxr    x1, [x2]
    0xc8, 0x5f, 0xfc, 0x41,          //ldaxr   x1, [x2]
    0xc8, 0xdf, 0xfc, 0x41,          //ldar    x1, [x2]
    0x88, 0x5f, 0x7c, 0x41,          //ldxr    w1, [x2]

    0x88, 0x5f, 0xfc, 0x41,          //ldaxr   w1, [x2]
    0x88, 0xdf, 0xfc, 0x41,          //ldar    w1, [x2]
    0xc8, 0x7f, 0x0c, 0x41,          //ldxp    x1, x3, [x2]
    0xc8, 0x7f, 0x8c, 0x41,          //ldaxp   x1, x3, [x2]

    0x88, 0x7f, 0x0c, 0x41,          //ldxp    w1, w3, [x2]
    0x88, 0x7f, 0x8c, 0x41,          //ldaxp   w1, w3, [x2]

    //pair
    0xa8,   0x40,   0x88,   0x61,        //ldnp    x1, x2, [x3,#8]
    0xa9,   0x40,   0x88,   0x61,        //ldp     x1, x2, [x3,#8]
    0xa9,   0xc0,   0x88,   0x61,        //ldp     x1, x2, [x3,#8]!
    0xa8,   0xc0,   0x88,   0x61,        //ldp     x1, x2, [x3],#8
    0xa8,   0xc0,   0x88,   0x61,        //ldp     x1, x2, [x3],#8

    //unsacled
    0x38,   0x40,   0x10,   0x61,        //ldurb   w1, [x3,#1]
    0x38,   0x80,   0x10,   0x61,        //ldursb  x1, [x3,#1]
    0xf8,   0x40,   0x10,   0x61,        //ldur     x1, [x3,#1]
    0x78,   0x40,   0x10,   0x61,        //ldurh    w1, [x3,#1]
    0x78,   0x80,   0x10,   0x61,        //ldursh   x1, [x3,#1]
    0xb8,   0x80,   0x10,   0x61,        //ldursw   x1, [x3,#1]

    //unprevlidged
    0x38,   0x40,   0x18,   0x61,        //ldtrb   w1, [x3,#1]
    0x38,   0x80,   0x18,   0x61,        //ldtrsb  x1, [x3,#1]
    0xf8,   0x40,   0x18,   0x61,        //ldtr    x1, [x3,#1]
    0x78,   0x40,   0x18,   0x61,        //ldtrh   w1, [x3,#1]
    0x78,   0x80,   0x18,   0x61,        //ldtrsh  x1, [x3,#1]
    0xb8,   0x80,   0x18,   0x61,        //ldtrsw  x1, [x3,#1]

    //----store----
    0xf8,   0x00,   0x14,   0x41,        //str     x1, [x2],#1
    0xf8,   0x0f,   0xf4,   0x41,        //str     x1, [x2],#255
    0x38,   0x00,   0x14,   0x41,        //strb    w1, [x2],#1
    0x78,   0x00,   0x14,   0x41,        //strh    w1, [x2],#1

    0xf9,   0x00,   0x04,   0x41,        //str     x1, [x2,#8]
    0xf9,   0x00,   0x80,   0x41,        //str     x1, [x2,#256]
    0x39,   0x00,   0x10,   0x41,        //strb    w1, [x2,#4]
    0x79,   0x00,   0x08,   0x41,        //strh    w1, [x2,#4]

    0xf8,   0x23,   0x68,   0x41,        //str     x1, [x2,x3]
    0xf8,   0x23,   0x68,   0x41,        //str     x1, [x2,x3]
    0x38,   0x23,   0x68,   0x41,        //strb    w1, [x2,x3]
    0x78,   0x23,   0x68,   0x41,        //strh    w1, [x2,x3]

    0xf8,   0x00,   0x1c,   0x41,        //str     x1, [x2,#1]!
    0xf8,   0x0f,   0xfc,   0x41,        //str     x1, [x2,#255]!
    0x38,   0x00,   0x1c,   0x41,        //strb    w1, [x2,#1]!
    0x78,   0x00,   0x1c,   0x41,        //strh    w1, [x2,#1]!

    0x08,   0x00,   0x7c,   0x41,        //stxrb   w0, w1, [x2]
    0x48,   0x00,   0x7c,   0x41,        //stxrh   w0, w1, [x2]
    0x88,   0x00,   0x7c,   0x41,        //stxr    w0, w1, [x2]
    0x88,   0x20,   0x0c,   0x41,        //stxp    w0, w1, w3, [x2]

    0xa8,   0x00,   0x88,   0x61,        //stnp    x1, x2, [x3,#8]
    0xa9,   0x00,   0x88,   0x61,        //stp     x1, x2, [x3,#8]
    0xa9,   0x80,   0x88,   0x61,        //stp     x1, x2, [x3,#8]!
    0xa8,   0x80,   0x88,   0x61,        //stp     x1, x2, [x3],#8
    0xa8,   0x80,   0x88,   0x61,        //stp     x1, x2, [x3],#8

    0x38,   0x00,   0x10,   0x61,        //sturb   w1, [x3,#1]
    0xf8,   0x00,   0x10,   0x61,        //str     x1, [x3,#1]
    0x78,   0x00,   0x10,   0x61,        //strh    w1, [x3,#1]

    0xf8,   0x00,   0x18,   0x61,        //sttr    x1, [x3,#1]
    0x38,   0x00,   0x18,   0x61,        //sttrb   w1, [x3,#1]
    0x78,   0x00,   0x18,   0x61,        //sttrh   w1, [x3,#1]

    0x08,   0x9f,   0xfc,   0x61,        //stlrb   w1, [x3]
    0xc8,   0x9f,   0xfc,   0x61,        //stlr    x1, [x3]
    0x48,   0x9f,   0xfc,   0x61,        //stlrh   w1, [x3]
    0xc8,   0x20,   0x88,   0x61,        //stlxp   w0, x1, x2, [x3]
    0x08,   0x00,   0xfc,   0x61,        //stlxrb  w0, w1, [x3]
    0xc8,   0x00,   0xfc,   0x61,        //stlxr   w0, x1, [x3]
    0x48,   0x00,   0xfc,   0x61,        //stlxrh  w0, w1, [x3]

    0xf8,   0x63,   0x68,   0x41,        //ldr     x1, [x2,x3]
    0xf8,   0x63,   0x78,   0x41,        //ldr     x1, [x2,x3,lsl #3]
    0xb8,   0x63,   0x78,   0x41,        //ldr     w1, [x2,x3,lsl #2]
    0xf8,   0x63,   0x48,   0x41,        //ldr     x1, [x2,w3,uxtw]

    0xf8,   0x63,   0xe8,   0x41,        //ldr     x1, [x2,x3,sxtx]
    0x9a,   0x82,   0x04,   0x20,        //csinc   x0, x1, x2, eq
    0xda,   0x82,   0x00,   0x20,        //csinv   x0, x1, x2, eq
    0xda,   0x82,   0x04,   0x20,        //csneg   x0, x1, x2, eq

    0xd3,   0x7f,   0x00,   0x20,        //ubfiz   x0, x1, #1, #1
    0x53,   0x00,   0x7c,   0x20,        //lsr     w0, w1, #0
    0x92,   0x9f,   0xff,   0xe0,        //mov     x0, #0xffffffffffff0000         // #-65536
    0x92,   0xff,   0xff,   0xe0,        //mov     x0, #0xffffffffffff             // #281474976710655

    0x12,   0xbf,   0xff,   0xe0,        //movn    w0, #0xffff, lsl #16
    0xd2,   0x9f,   0xff,   0xe0,        //mov     x0, #0xffff                     // #65535
    0xd2,   0xff,   0xff,   0xe0,        //mov     x0, #0xffff000000000000         // #-281474976710656
    0x52,   0xbf,   0xff,   0xe0,        //mov     w0, #0xffff0000                 // #-65536

    0x18,   0x7f,   0xff,   0xbe,        //ldr     w30, 500630 0xffff4
    0x58,   0xf3,   0xcb,   0x1e,        //ldr     x30, 3e7fa0  -0x184d0
    0xb8,   0x4f,   0xf4,   0x0f,        //ldr     w15, [x0],#255
    0xf8,   0x51,   0x07,   0xcf,        //ldr     x15, [x30],#-240

    0xb9,   0x7f,   0xff,   0xfd,        //ldr     w29, [sp,#16380]
    0xf9,   0x7f,   0xff,   0xfd,        //ldr     x29, [sp,#32760]
    0xf8,   0x7f,   0x68,   0x41,        //ldr     x1, [x2,x31]
    0xf8,   0x4f,   0xff,   0xe1,        //ldr     x1, [sp,#255]!

    //prfm
    0xd8,   0x00,   0x00,   0x3f,	//prfm	   1F, [pc + 4]
    0xd8,   0xff,   0xff,   0xe0,	//prfm	   0, [pc + fffffffffffffffc]
    0xd8,   0x0f,   0xd8,   0x08,	//prfm	   8, [pc + 1fb00]
    0xf9,   0x80,   0x04,   0x3d,	//prfm	   1d, [x1 + 8]
    0xf9,   0xbf,   0xff,   0xdf,	//prfm	   1f, [x30 + 7ff8]
    0xf8,   0xa5,   0x68,   0x44,	//prfm	   4, [x2 + x5 << 0]
    0xf8,   0xa1,   0x5b,   0xc5,	//prfm	   5, [x30 + w1 << 3]
    0xf8,   0xa5,   0xe9,   0x06,	//prfm	   6, [x8 + x5 << 0]
    
    0xd5,   0x03,   0x20,   0x1f,        //nop
  };
  // clang-format on

  reverseBuffer(buffer.data(), buffer.size());
  InstructionDecoder d(buffer.data(), buffer.size(), Dyninst::Arch_aarch64);

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

  RegisterAST::Ptr x0 = create(aarch64::x0);
  RegisterAST::Ptr x1 = create(aarch64::x1);
  RegisterAST::Ptr x2 = create(aarch64::x2);
  RegisterAST::Ptr x3 = create(aarch64::x3);
  RegisterAST::Ptr x4 = create(aarch64::x4);
  RegisterAST::Ptr x5 = create(aarch64::x5);
  RegisterAST::Ptr x6 = create(aarch64::x6);
  RegisterAST::Ptr x7 = create(aarch64::x7);
  RegisterAST::Ptr x8 = create(aarch64::x8);
  RegisterAST::Ptr x9 = create(aarch64::x9);
  RegisterAST::Ptr x10 = create(aarch64::x10);
  RegisterAST::Ptr x11 = create(aarch64::x11);
  RegisterAST::Ptr x12 = create(aarch64::x12);
  RegisterAST::Ptr x13 = create(aarch64::x13);
  RegisterAST::Ptr x14 = create(aarch64::x14);
  RegisterAST::Ptr x15 = create(aarch64::x15);
  RegisterAST::Ptr x16 = create(aarch64::x16);
  RegisterAST::Ptr x17 = create(aarch64::x17);
  RegisterAST::Ptr x18 = create(aarch64::x18);
  RegisterAST::Ptr x19 = create(aarch64::x19);
  RegisterAST::Ptr x20 = create(aarch64::x20);
  RegisterAST::Ptr x21 = create(aarch64::x21);
  RegisterAST::Ptr x22 = create(aarch64::x22);
  RegisterAST::Ptr x23 = create(aarch64::x23);
  RegisterAST::Ptr x24 = create(aarch64::x24);
  RegisterAST::Ptr x25 = create(aarch64::x25);
  RegisterAST::Ptr x26 = create(aarch64::x26);
  RegisterAST::Ptr x27 = create(aarch64::x27);
  RegisterAST::Ptr x28 = create(aarch64::x28);
  RegisterAST::Ptr x29 = create(aarch64::x29);
  RegisterAST::Ptr x30 = create(aarch64::x30);

  RegisterAST::Ptr w0 = create(aarch64::w0);
  RegisterAST::Ptr w1 = create(aarch64::w1);
  RegisterAST::Ptr w2 = create(aarch64::w2);
  RegisterAST::Ptr w3 = create(aarch64::w3);
  RegisterAST::Ptr w4 = create(aarch64::w4);
  RegisterAST::Ptr w5 = create(aarch64::w5);
  RegisterAST::Ptr w6 = create(aarch64::w6);
  RegisterAST::Ptr w7 = create(aarch64::w7);
  RegisterAST::Ptr w8 = create(aarch64::w8);
  RegisterAST::Ptr w9 = create(aarch64::w9);
  RegisterAST::Ptr w10 = create(aarch64::w10);
  RegisterAST::Ptr w11 = create(aarch64::w11);
  RegisterAST::Ptr w12 = create(aarch64::w12);
  RegisterAST::Ptr w13 = create(aarch64::w13);
  RegisterAST::Ptr w14 = create(aarch64::w14);
  RegisterAST::Ptr w15 = create(aarch64::w15);
  RegisterAST::Ptr w16 = create(aarch64::w16);
  RegisterAST::Ptr w17 = create(aarch64::w17);
  RegisterAST::Ptr w18 = create(aarch64::w18);
  RegisterAST::Ptr w19 = create(aarch64::w19);
  RegisterAST::Ptr w20 = create(aarch64::w20);
  RegisterAST::Ptr w21 = create(aarch64::w21);
  RegisterAST::Ptr w22 = create(aarch64::w22);
  RegisterAST::Ptr w23 = create(aarch64::w23);
  RegisterAST::Ptr w24 = create(aarch64::w24);
  RegisterAST::Ptr w25 = create(aarch64::w25);
  RegisterAST::Ptr w26 = create(aarch64::w26);
  RegisterAST::Ptr w27 = create(aarch64::w27);
  RegisterAST::Ptr w28 = create(aarch64::w28);
  RegisterAST::Ptr w29 = create(aarch64::w29);
  RegisterAST::Ptr w30 = create(aarch64::w30);

  RegisterAST::Ptr q0 = create(aarch64::q0);
  RegisterAST::Ptr q1 = create(aarch64::q1);
  RegisterAST::Ptr q2 = create(aarch64::q2);
  RegisterAST::Ptr q3 = create(aarch64::q3);
  RegisterAST::Ptr q4 = create(aarch64::q4);
  RegisterAST::Ptr q5 = create(aarch64::q5);
  RegisterAST::Ptr q6 = create(aarch64::q6);
  RegisterAST::Ptr q7 = create(aarch64::q7);
  RegisterAST::Ptr q8 = create(aarch64::q8);
  RegisterAST::Ptr q9 = create(aarch64::q9);
  RegisterAST::Ptr q10 = create(aarch64::q10);
  RegisterAST::Ptr q11 = create(aarch64::q11);
  RegisterAST::Ptr q12 = create(aarch64::q12);
  RegisterAST::Ptr q13 = create(aarch64::q13);
  RegisterAST::Ptr q14 = create(aarch64::q14);
  RegisterAST::Ptr q15 = create(aarch64::q15);
  RegisterAST::Ptr q16 = create(aarch64::q16);
  RegisterAST::Ptr q17 = create(aarch64::q17);
  RegisterAST::Ptr q18 = create(aarch64::q18);
  RegisterAST::Ptr q19 = create(aarch64::q19);
  RegisterAST::Ptr q20 = create(aarch64::q20);
  RegisterAST::Ptr q21 = create(aarch64::q21);
  RegisterAST::Ptr q22 = create(aarch64::q22);
  RegisterAST::Ptr q23 = create(aarch64::q23);
  RegisterAST::Ptr q24 = create(aarch64::q24);
  RegisterAST::Ptr q25 = create(aarch64::q25);
  RegisterAST::Ptr q26 = create(aarch64::q26);
  RegisterAST::Ptr q27 = create(aarch64::q27);
  RegisterAST::Ptr q28 = create(aarch64::q28);
  RegisterAST::Ptr q29 = create(aarch64::q29);
  RegisterAST::Ptr q30 = create(aarch64::q30);
  RegisterAST::Ptr q31 = create(aarch64::q31);

  RegisterAST::Ptr s0 = create(aarch64::s0);
  RegisterAST::Ptr s1 = create(aarch64::s1);
  RegisterAST::Ptr s2 = create(aarch64::s2);
  RegisterAST::Ptr s3 = create(aarch64::s3);
  RegisterAST::Ptr s4 = create(aarch64::s4);
  RegisterAST::Ptr s5 = create(aarch64::s5);
  RegisterAST::Ptr s6 = create(aarch64::s6);
  RegisterAST::Ptr s7 = create(aarch64::s7);
  RegisterAST::Ptr s8 = create(aarch64::s8);
  RegisterAST::Ptr s9 = create(aarch64::s9);
  RegisterAST::Ptr s10 = create(aarch64::s10);
  RegisterAST::Ptr s11 = create(aarch64::s11);
  RegisterAST::Ptr s12 = create(aarch64::s12);
  RegisterAST::Ptr s13 = create(aarch64::s13);
  RegisterAST::Ptr s14 = create(aarch64::s14);
  RegisterAST::Ptr s15 = create(aarch64::s15);
  RegisterAST::Ptr s16 = create(aarch64::s16);
  RegisterAST::Ptr s17 = create(aarch64::s17);
  RegisterAST::Ptr s18 = create(aarch64::s18);
  RegisterAST::Ptr s19 = create(aarch64::s19);
  RegisterAST::Ptr s20 = create(aarch64::s20);
  RegisterAST::Ptr s21 = create(aarch64::s21);
  RegisterAST::Ptr s22 = create(aarch64::s22);
  RegisterAST::Ptr s23 = create(aarch64::s23);
  RegisterAST::Ptr s24 = create(aarch64::s24);
  RegisterAST::Ptr s25 = create(aarch64::s25);
  RegisterAST::Ptr s26 = create(aarch64::s26);
  RegisterAST::Ptr s27 = create(aarch64::s27);
  RegisterAST::Ptr s28 = create(aarch64::s28);
  RegisterAST::Ptr s29 = create(aarch64::s29);
  RegisterAST::Ptr s30 = create(aarch64::s30);
  RegisterAST::Ptr s31 = create(aarch64::s31);

  RegisterAST::Ptr h0 = create(aarch64::h0);
  RegisterAST::Ptr h1 = create(aarch64::h1);
  RegisterAST::Ptr h2 = create(aarch64::h2);
  RegisterAST::Ptr h3 = create(aarch64::h3);
  RegisterAST::Ptr h4 = create(aarch64::h4);
  RegisterAST::Ptr h5 = create(aarch64::h5);
  RegisterAST::Ptr h6 = create(aarch64::h6);
  RegisterAST::Ptr h7 = create(aarch64::h7);
  RegisterAST::Ptr h8 = create(aarch64::h8);
  RegisterAST::Ptr h9 = create(aarch64::h9);
  RegisterAST::Ptr h10 = create(aarch64::h10);
  RegisterAST::Ptr h11 = create(aarch64::h11);
  RegisterAST::Ptr h12 = create(aarch64::h12);
  RegisterAST::Ptr h13 = create(aarch64::h13);
  RegisterAST::Ptr h14 = create(aarch64::h14);
  RegisterAST::Ptr h15 = create(aarch64::h15);
  RegisterAST::Ptr h16 = create(aarch64::h16);
  RegisterAST::Ptr h17 = create(aarch64::h17);
  RegisterAST::Ptr h18 = create(aarch64::h18);
  RegisterAST::Ptr h19 = create(aarch64::h19);
  RegisterAST::Ptr h20 = create(aarch64::h20);
  RegisterAST::Ptr h21 = create(aarch64::h21);
  RegisterAST::Ptr h22 = create(aarch64::h22);
  RegisterAST::Ptr h23 = create(aarch64::h23);
  RegisterAST::Ptr h24 = create(aarch64::h24);
  RegisterAST::Ptr h25 = create(aarch64::h25);
  RegisterAST::Ptr h26 = create(aarch64::h26);
  RegisterAST::Ptr h27 = create(aarch64::h27);
  RegisterAST::Ptr h28 = create(aarch64::h28);
  RegisterAST::Ptr h29 = create(aarch64::h29);
  RegisterAST::Ptr h30 = create(aarch64::h30);
  RegisterAST::Ptr h31 = create(aarch64::h31);

  RegisterAST::Ptr d0 = create(aarch64::d0);
  RegisterAST::Ptr d1 = create(aarch64::d1);
  RegisterAST::Ptr d2 = create(aarch64::d2);
  RegisterAST::Ptr d3 = create(aarch64::d3);
  RegisterAST::Ptr d4 = create(aarch64::d4);
  RegisterAST::Ptr d5 = create(aarch64::d5);
  RegisterAST::Ptr d6 = create(aarch64::d6);
  RegisterAST::Ptr d7 = create(aarch64::d7);
  RegisterAST::Ptr d8 = create(aarch64::d8);
  RegisterAST::Ptr d9 = create(aarch64::d9);
  RegisterAST::Ptr d10 = create(aarch64::d10);
  RegisterAST::Ptr d11 = create(aarch64::d11);
  RegisterAST::Ptr d12 = create(aarch64::d12);
  RegisterAST::Ptr d13 = create(aarch64::d13);
  RegisterAST::Ptr d14 = create(aarch64::d14);
  RegisterAST::Ptr d15 = create(aarch64::d15);
  RegisterAST::Ptr d16 = create(aarch64::d16);
  RegisterAST::Ptr d17 = create(aarch64::d17);
  RegisterAST::Ptr d18 = create(aarch64::d18);
  RegisterAST::Ptr d19 = create(aarch64::d19);
  RegisterAST::Ptr d20 = create(aarch64::d20);
  RegisterAST::Ptr d21 = create(aarch64::d21);
  RegisterAST::Ptr d22 = create(aarch64::d22);
  RegisterAST::Ptr d23 = create(aarch64::d23);
  RegisterAST::Ptr d24 = create(aarch64::d24);
  RegisterAST::Ptr d25 = create(aarch64::d25);
  RegisterAST::Ptr d26 = create(aarch64::d26);
  RegisterAST::Ptr d27 = create(aarch64::d27);
  RegisterAST::Ptr d28 = create(aarch64::d28);
  RegisterAST::Ptr d29 = create(aarch64::d29);
  RegisterAST::Ptr d30 = create(aarch64::d30);
  RegisterAST::Ptr d31 = create(aarch64::d31);

  RegisterAST::Ptr b0 = create(aarch64::b0);
  RegisterAST::Ptr b1 = create(aarch64::b1);
  RegisterAST::Ptr b2 = create(aarch64::b2);
  RegisterAST::Ptr b3 = create(aarch64::b3);
  RegisterAST::Ptr b4 = create(aarch64::b4);
  RegisterAST::Ptr b5 = create(aarch64::b5);
  RegisterAST::Ptr b6 = create(aarch64::b6);
  RegisterAST::Ptr b7 = create(aarch64::b7);
  RegisterAST::Ptr b8 = create(aarch64::b8);
  RegisterAST::Ptr b9 = create(aarch64::b9);
  RegisterAST::Ptr b10 = create(aarch64::b10);
  RegisterAST::Ptr b11 = create(aarch64::b11);
  RegisterAST::Ptr b12 = create(aarch64::b12);
  RegisterAST::Ptr b13 = create(aarch64::b13);
  RegisterAST::Ptr b14 = create(aarch64::b14);
  RegisterAST::Ptr b15 = create(aarch64::b15);
  RegisterAST::Ptr b16 = create(aarch64::b16);
  RegisterAST::Ptr b17 = create(aarch64::b17);
  RegisterAST::Ptr b18 = create(aarch64::b18);
  RegisterAST::Ptr b19 = create(aarch64::b19);
  RegisterAST::Ptr b20 = create(aarch64::b20);
  RegisterAST::Ptr b21 = create(aarch64::b21);
  RegisterAST::Ptr b22 = create(aarch64::b22);
  RegisterAST::Ptr b23 = create(aarch64::b23);
  RegisterAST::Ptr b24 = create(aarch64::b24);
  RegisterAST::Ptr b25 = create(aarch64::b25);
  RegisterAST::Ptr b26 = create(aarch64::b26);
  RegisterAST::Ptr b27 = create(aarch64::b27);
  RegisterAST::Ptr b28 = create(aarch64::b28);
  RegisterAST::Ptr b29 = create(aarch64::b29);
  RegisterAST::Ptr b30 = create(aarch64::b30);
  RegisterAST::Ptr b31 = create(aarch64::b31);

  RegisterAST::Ptr xzr = create(aarch64::xzr);
  RegisterAST::Ptr wzr = create(aarch64::wzr);
  RegisterAST::Ptr sp = create(aarch64::sp);
  RegisterAST::Ptr wsp = create(aarch64::wsp);
  RegisterAST::Ptr pc = create(aarch64::pc);
  RegisterAST::Ptr pstate = create(aarch64::pstate);

  std::vector<registerSet> expectedRead, expectedWritten;

  // ldr x1, #4
  expectedRead.push_back({pc});
  expectedWritten.push_back({x1});

  // ldr x1, #256
  expectedRead.push_back({pc});
  expectedWritten.push_back({x1});

  // ldr w1, #4
  expectedRead.push_back({pc});
  expectedWritten.push_back({w1});

  // ldr w1, #1048576
  expectedRead.push_back({pc});
  expectedWritten.push_back({w1});

  // ldr, x1, [x2], #1
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1});

  // ldr, x1, [x2], #255
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1});

  // ldrb, w1, [x2], #1
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldrsb, w1, [x2], #1
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldrh, w1, [x2], #1
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldrsh, x1, [x2], #1
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldr, x1, [x2, #8]
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1});

  // ldr, x1, [x2, #256]
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1});

  // ldrb, w1, [x2, #4]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldrsb, w1, [x2, #4]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldrh, w1, [x2, #4]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldrsh, w1, [x2, #4]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldrsw, x1, [x2, #4]
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1});

  // ldr, x1, [x2, x3]
  expectedRead.push_back({x2, x3});
  expectedWritten.push_back({x1});

  // ldrb, w1, [x2, x3]
  expectedRead.push_back({x2, x3});
  expectedWritten.push_back({w1});

  // ldrsb, w1, [x2, x3]
  expectedRead.push_back({x2, x3});
  expectedWritten.push_back({w1});

  // ldrh, w1, [x2, x3]
  expectedRead.push_back({x2, x3});
  expectedWritten.push_back({w1});

  // ldrsh, w1, [x2, x3]
  expectedRead.push_back({x2, x3});
  expectedWritten.push_back({w1});

  // ldrsw, x1, [x2, x3]
  expectedRead.push_back({x2, x3});
  expectedWritten.push_back({x1});

  // ldr, x1, [x2, #1]!
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1});

  // ldr, x1, [x2, #255]!
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1});

  // ldrb, w1, [x2, #1]!
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldrsb, w1, [x2, #1]!
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldrh, w1, [x2, #1]!
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldrsh, w1, [x2, #1]!
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldrsw, x1, [x2, #1]!
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1});

  // ldar x1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1});

  // ldar w1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldxrb w1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldaxrb w1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldarb w1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldxrh w1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldaxrh w1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldarh w1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldxr x1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1});

  // ldaxr x1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1});

  // ldar x1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1});

  // ldxr w1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldaxr w1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldar w1, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1});

  // ldxp x1, x3, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1, x3});

  // ldaxp x1, x3, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({x1, x3});

  // ldxp w1, w3, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1, w3});

  // ldaxp w1, w3, [x2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({w1, w3});

  // ldnp x1, x2, [x3,#8]
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1, x2});

  // ldp x1, x2, [x3,#8]
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1, x2});

  // ldp x1, x2, [x3,#8]!
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1, x2});

  // ldp x1, x2, [x3],#8
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1, x2});

  // ldp x1, x2, [x3],#8
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1, x2});

  // ldurb w1, [x3,#1]
  expectedRead.push_back({x3});
  expectedWritten.push_back({w1});

  // ldursb x1, [x3,#1]
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1});

  // ldur x1, [x3,#1]
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1});

  // ldurh w1, [x3,#1]
  expectedRead.push_back({x3});
  expectedWritten.push_back({w1});

  // ldursh x1, [x3,#1]
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1});

  // ldursw x1, [x3,#1]
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1});

  // ldtrb w1, [x3,#1]
  expectedRead.push_back({x3});
  expectedWritten.push_back({w1});

  // ldtrsb x1, [x3,#1]
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1});

  // ldtr x1, [x3,#1]
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1});

  // ldtrh w1, [x3,#1]
  expectedRead.push_back({x3});
  expectedWritten.push_back({w1});

  // ldtrsh x1, [x3,#1]
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1});

  // ldtrsw x1, [x3,#1]
  expectedRead.push_back({x3});
  expectedWritten.push_back({x1});

  // str x1, [x2],#1
  expectedRead.push_back({x2, x1});
  expectedWritten.push_back({});

  // str x1, [x2],#255
  expectedRead.push_back({x2, x1});
  expectedWritten.push_back({});

  // strb w1, [x2],#1
  expectedRead.push_back({x2, w1});
  expectedWritten.push_back({});

  // strh w1, [x2],#1
  expectedRead.push_back({x2, w1});
  expectedWritten.push_back({});

  // str x1, [x2,#8]
  expectedRead.push_back({x2, x1});
  expectedWritten.push_back({});

  // str x1, [x2,#256]
  expectedRead.push_back({x2, x1});
  expectedWritten.push_back({});

  // strb w1, [x2,#4]
  expectedRead.push_back({x2, w1});
  expectedWritten.push_back({});

  // strh w1, [x2,#4]
  expectedRead.push_back({x2, w1});
  expectedWritten.push_back({});

  // str x1, [x2,x3]
  expectedRead.push_back({x2, x1, x3});
  expectedWritten.push_back({});

  // str x1, [x2,x3]
  expectedRead.push_back({x2, x1, x3});
  expectedWritten.push_back({});

  // strb w1, [x2,x3]
  expectedRead.push_back({x2, w1, x3});
  expectedWritten.push_back({});

  // strh w1, [x2,x3]
  expectedRead.push_back({x2, w1, x3});
  expectedWritten.push_back({});

  // str x1, [x2,#1]!
  expectedRead.push_back({x2, x1});
  expectedWritten.push_back({});

  // str x1, [x2,#255]!
  expectedRead.push_back({x2, x1});
  expectedWritten.push_back({});

  // strb w1, [x2,#1]!
  expectedRead.push_back({x2, w1});
  expectedWritten.push_back({});

  // strh w1, [x2,#1]!
  expectedRead.push_back({x2, w1});
  expectedWritten.push_back({});

  // stxrb w0, w1, [x2]
  expectedRead.push_back({x2, w1});
  expectedWritten.push_back({w0});

  // stxrh w0, w1, [x2]
  expectedRead.push_back({x2, w1});
  expectedWritten.push_back({w0});

  // stxr w0, w1, [x2]
  expectedRead.push_back({x2, w1});
  expectedWritten.push_back({w0});

  // stxp w0, w1, w3, [x2]
  expectedRead.push_back({x2, w1, w3});
  expectedWritten.push_back({w0});

  // stnp x1, x2, [x3,#8]
  expectedRead.push_back({x2, x1, x3});
  expectedWritten.push_back({});

  // stp x1, x2, [x3,#8]
  expectedRead.push_back({x2, x1, x3});
  expectedWritten.push_back({});

  // stp x1, x2, [x3,#8]!
  expectedRead.push_back({x2, x1, x3});
  expectedWritten.push_back({});

  // stp x1, x2, [x3],#8
  expectedRead.push_back({x2, x1, x3});
  expectedWritten.push_back({});

  // stp x1, x2, [x3],#8
  expectedRead.push_back({x2, x1, x3});
  expectedWritten.push_back({});

  // sturb w1, [x3,#1]
  expectedRead.push_back({x3, w1});
  expectedWritten.push_back({});

  // str x1, [x3,#1]
  expectedRead.push_back({x3, x1});
  expectedWritten.push_back({});

  // strh w1, [x3,#1]
  expectedRead.push_back({x3, w1});
  expectedWritten.push_back({});

  // sttr x1, [x3,#1]
  expectedRead.push_back({x3, x1});
  expectedWritten.push_back({});

  // sttrb w1, [x3,#1]
  expectedRead.push_back({x3, w1});
  expectedWritten.push_back({});

  // sttrh w1, [x3,#1]
  expectedRead.push_back({x3, w1});
  expectedWritten.push_back({});

  // stlrb w1, [x3]
  expectedRead.push_back({x3, w1});
  expectedWritten.push_back({});

  // stlr x1, [x3]
  expectedRead.push_back({x3, x1});
  expectedWritten.push_back({});

  // stlrh w1, [x3]
  expectedRead.push_back({x3, w1});
  expectedWritten.push_back({});

  // stlxp w0, x1, x2, [x3]
  expectedRead.push_back({x3, x1, x2});
  expectedWritten.push_back({w0});

  // stlxrb w0, w1, [x3]
  expectedRead.push_back({w1, x3});
  expectedWritten.push_back({w0});

  // stlxr w0, x1, [x3]
  expectedRead.push_back({x1, x3});
  expectedWritten.push_back({w0});

  // stlxrh w0, w1, [x3]
  expectedRead.push_back({w1, x3});
  expectedWritten.push_back({w0});

  // ldr x1, [x2,x3]
  expectedRead.push_back({x3, x2});
  expectedWritten.push_back({x1});

  // ldr x1, [x2,x3,lsl #3]
  expectedRead.push_back({x3, x2});
  expectedWritten.push_back({x1});

  // ldr w1, [x2,x3,lsl #2]
  expectedRead.push_back({x3, x2});
  expectedWritten.push_back({w1});

  // ldr x1, [x2,w3,uxtw]
  expectedRead.push_back({w3, x2});
  expectedWritten.push_back({x1});

  // ldr x1, [x2,x3,sxtx]
  expectedRead.push_back({x3, x2});
  expectedWritten.push_back({x1});

  // csinc x0, x1, x2, eq
  expectedRead.push_back({x1, x2, pstate});
  expectedWritten.push_back({x0});

  // csinv x0, x1, x2, eq
  expectedRead.push_back({x1, x2, pstate});
  expectedWritten.push_back({x0});

  // csneg x0, x1, x2, eq
  expectedRead.push_back({x1, x2, pstate});
  expectedWritten.push_back({x0});

  // ubfiz x0, x1, #1, #1
  expectedRead.push_back({x1});
  expectedWritten.push_back({x0});

  // lsr w0, w1, #0
  expectedRead.push_back({w1});
  expectedWritten.push_back({w0});

  // mov x0, #0xffffffffffff0000
  expectedRead.push_back({});
  expectedWritten.push_back({x0});

  // mov x0, #0xffffffffffff
  expectedRead.push_back({});
  expectedWritten.push_back({x0});

  // movn w0, #0xffff, lsl #16
  expectedRead.push_back({});
  expectedWritten.push_back({w0});

  // mov x0, #0xffff
  expectedRead.push_back({});
  expectedWritten.push_back({x0});

  // mov x0, #0xffff000000000000
  expectedRead.push_back({});
  expectedWritten.push_back({x0});

  // mov w0, #0xffff0000
  expectedRead.push_back({});
  expectedWritten.push_back({w0});

  // ldr w30, 500630 0xffff4
  expectedRead.push_back({pc});
  expectedWritten.push_back({w30});

  // ldr x30, 3e7fa0 -0x184d0
  expectedRead.push_back({pc});
  expectedWritten.push_back({x30});

  // ldr w15, [x0],#255
  expectedRead.push_back({x0});
  expectedWritten.push_back({w15});

  // ldr x15, [x30],#-240
  expectedRead.push_back({x30});
  expectedWritten.push_back({x15});

  // ldr w29, [sp,#16380]
  expectedRead.push_back({sp});
  expectedWritten.push_back({w29});

  // ldr x29, [sp,#32760]
  expectedRead.push_back({sp});
  expectedWritten.push_back({x29});

  // ldr x1, [x2,x31]
  expectedRead.push_back({x2, xzr});
  expectedWritten.push_back({x1});

  // ldr x1, [sp,#255]!
  expectedRead.push_back({sp});
  expectedWritten.push_back({x1});

  // prfm 1F, [pc + 4]
  expectedRead.push_back({pc});
  expectedWritten.push_back({});

  // prfm 0, [pc + fffffffffffffffc]
  expectedRead.push_back({pc});
  expectedWritten.push_back({});

  // prfm 8, [pc + 1fb00]
  expectedRead.push_back({pc});
  expectedWritten.push_back({});

  // prfm 1d, [x1 + 8]
  expectedRead.push_back({x1});
  expectedWritten.push_back({});

  // prfm 1f, [x30 + 7ff8]
  expectedRead.push_back({x30});
  expectedWritten.push_back({});

  // prfm 4, [x2 + x5 << 0]
  expectedRead.push_back({x2, x5});
  expectedWritten.push_back({});

  // prfm 5, [x30 + w1 << 3]
  expectedRead.push_back({x30, w1});
  expectedWritten.push_back({});

  // prfm 6, [x8 + x5 << 0]
  expectedRead.push_back({x8, x5});
  expectedWritten.push_back({});

  // nop
  expectedRead.push_back({});
  expectedWritten.push_back({});

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

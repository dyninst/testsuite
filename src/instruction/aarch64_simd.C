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
#include <deque>
#include <vector>

using namespace Dyninst;
using namespace InstructionAPI;

class aarch64_simd_Mutator : public InstructionMutator {
public:
  aarch64_simd_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* aarch64_simd_factory() {
  return new aarch64_simd_Mutator();
}

void reverseBuffer(unsigned char* buffer, int bufferSize) {
  int elementCount = bufferSize / 4;

  for(int loop_index = 0; loop_index < elementCount; loop_index++) {
    std::swap(buffer[0], buffer[3]);
    std::swap(buffer[1], buffer[2]);
    buffer += 4;
  }
}

test_results_t aarch64_simd_Mutator::executeTest() {
  constexpr auto num_tests = 131;

  // clang-format off
  std::array<uint8_t, 4*num_tests> buffer = {
    0x4e, 0x30, 0x38, 0x20,     // saddlv  h0, q1.8b
    0x0e, 0x30, 0xa9, 0x0f,     // smaxv   b15, d8
    0x6e, 0x70, 0x38, 0x42,     // uaddlv  s2, q2
    0x2e, 0x31, 0xa8, 0x05,     // uminv   b5, d0
    0x6e, 0x30, 0xc8, 0x25,     // fmaxnmv s5, q1
    0x0e, 0x11, 0x04, 0x41,     // dup     d1, q2
    0x4e, 0x02, 0x0c, 0xd8,     // dup     q20, w5
    0x0e, 0x18, 0x0c, 0x40,     // dup     d0, x2
    0x4e, 0x08, 0x1c, 0x04,     // ins     q4, x0
    0x4e, 0x01, 0x1f, 0xff,     // ins     q31, wzr
    0x0e, 0x05, 0x2c, 0xa1,     // smov    w1, d5
    0x4e, 0x10, 0x3c, 0x42,     // umov    x2, q2
    0x6e, 0x01, 0x04, 0x00,     // ins     q0, d0
    0x2e, 0x02, 0x28, 0x20,     // ext     d0, d1, d2, #5
    0x6e, 0x02, 0x7a, 0x08,     // ext     q8, q16, q2, #15
    0x0f, 0x00, 0xe5, 0x00,     // movi    d0, #8
    0x4f, 0x07, 0xa7, 0xe1,     // movi    q1, ff lsl #8
    0x0f, 0x00, 0x74, 0x05,     // orr     d5, #0 lsl #24
    0x0f, 0x00, 0xb4, 0x05,     // orr     d5, #0 lsl #8
    0x4f, 0x07, 0xf7, 0xe8,     // fmov    q8, ff
    0x6f, 0x00, 0xc5, 0x80,     // mvni    q0, #12 lsl #8
    0x6f, 0x00, 0x97, 0x08,     // bic     d8, #24 lsl #0
    0x2f, 0x07, 0xe7, 0xe2,     // movi    d2, (all ones)
    0x0e, 0x00, 0x1a, 0x08,     // uzp1    d8, d16, d0
    0x4e, 0x04, 0x28, 0x62,     // trn1    q2, q3, q4
    0x0e, 0x08, 0x79, 0x08,     // zip2    d8, d8, d8
    0x5e, 0x11, 0x04, 0x25,     // dup     b5, q1
    0x5e, 0x08, 0x05, 0x08,     // dup     d8, d8
    0x5e, 0xf1, 0xb8, 0xa2,     // addp    d2, q5
    0x7e, 0x30, 0xc8, 0x81,     // fmaxnmp s1, d4
    0x7e, 0xf0, 0xc9, 0xef,     // fminnmp d15, q15
    0x5f, 0x78, 0x04, 0x82,     // sshr    d2, d4, #8
    0x5f, 0x40, 0x14, 0x20,     // ssra    d0, d1, #64
    0x5f, 0x41, 0x57, 0xff,     // shl     d31, d31, #1
    0x5f, 0x0e, 0x76, 0x08,     // sqshl   b8, b16, 6
    0x5f, 0x10, 0x9d, 0x02,     // sqrshrn h2, h8, #16
    0x5f, 0x20, 0xfc, 0x00,     // fcvtzs  s0, s0, #32
    0x7f, 0x48, 0x55, 0x04,     // sli     d4, d8, #8
    0x7f, 0x09, 0x94, 0x42,     // uqshrn  b2, h2, #7
    0x5e, 0x60, 0x91, 0x08,     // sqdmlal s8, h8, h0
    0x5e, 0xa1, 0xb0, 0x82,     // sqdmlsl d2, s4, s1
    0x5e, 0x7f, 0xd0, 0x00,     // sqdmull s0, h0, h31
    0x5e, 0xe0, 0x34, 0x22,     // cmgt    d2, d1, d0
    0x5e, 0x64, 0x4c, 0x40,     // sqshl   h0, h2, h4
    0x5e, 0xa0, 0xb4, 0x3f,     // sqdmulh s31, s1, s0
    0x5e, 0x68, 0xfd, 0x08,     // frecps  d8, d8, d8
    0x7e, 0xe2, 0x57, 0xe4,     // urshl   d4, d31, d2
    0x7e, 0xb0, 0xe5, 0x02,     // fcmgt   s2, s8, s16
    0x5e, 0x20, 0x39, 0x02,     // suqadd  b2, b8
    0x5e, 0x61, 0x48, 0x04,     // sqxtn   h4, s0
    0x5e, 0x61, 0xb8, 0x3f,     // fcvtms  d31, d1
    0x5e, 0xa1, 0xd8, 0xa9,     // frecpe  s9, s5
    0x7e, 0x21, 0x28, 0x48,     // sqxtun  b8, h2
    0x7e, 0x61, 0x68, 0x4f,     // fcvtxn  s15, d2
    0x7e, 0xa1, 0xdb, 0xff,     // frsqrte s31, s31
    0x5f, 0x49, 0x38, 0xa2,     // sqdmlal s2, h5, q9
    0x5f, 0x9f, 0x73, 0xe0,     // sqdmlsl d0, s31, d31
    0x5f, 0x4f, 0xd0, 0x88,     // sqrdmulh h8, h4, d15
    0x5f, 0x94, 0x18, 0x49,     // fmla    s9, s2, q20
    0x5f, 0xd1, 0x50, 0xa8,     // fmls    d8, d5, d17
    0x4e, 0x09, 0x01, 0x04,     // tbl     q4, q8, q9
    0x0e, 0x03, 0x30, 0x20,     // tbx     d0, d1, d2, d3
    0x4e, 0x1f, 0x43, 0x00,     // tbl     q0, q24, q25, q26, q31
    0x0e, 0x15, 0x70, 0x14,     // tbx     d20, d0, d1, d2, d3, d21
    0x4e, 0x23, 0x00, 0x41,     // saddl   q1, hq2, hq3
    0x0e, 0x25, 0x20, 0x9f,     // ssubl   q31, d4, d5
    0x4e, 0x30, 0x41, 0x02,     // addhn   hq2, q8, q16
    0x0e, 0x68, 0x70, 0x45,     // sabdl   q5, d2, d8
    0x4e, 0xa2, 0xb0, 0x20,     // sqdmlsl q0, hq1, hq2
    0x2e, 0x3f, 0x61, 0xe2,     // rsubhn  d2, q15, q31
    0x4e, 0x63, 0x60, 0x48,     // subhn   hq8, q2,q3
    0x0e, 0x25, 0x0c, 0x62,     // sqadd   d2, d3, d5
    0x4e, 0x3f, 0x34, 0x20,     // cmgt    q0, q1, q31
    0x0e, 0x2a, 0x7d, 0x28,     // saba    d8, d9, d10
    0x6e, 0x66, 0x1c, 0xa4,     // bsl     q4, q5, q6
    0x0e, 0x20, 0x59, 0x28,     // cnt     d8, d9
    0x4e, 0x21, 0x2b, 0xe0,     // xtn     q0, q31
    0x4e, 0x61, 0xc8, 0x62,     // fcvtas  q2, q3
    0x2e, 0xa1, 0xf9, 0xff,     // fsqrt   d31, d15
    0x0f, 0x40, 0x20, 0xa2,     // smlal   q2, d5, d0
    0x4f, 0x85, 0x6b, 0xe1,     // smlsl   q1, hq31, q5
    0x4f, 0x9f, 0x80, 0x49,     // mul     q9, q2, d31
    0x0f, 0x45, 0xb8, 0x88,     // sqdmull q8, d4, q5
    0x0f, 0x83, 0xd0, 0x41,     // sqrdmulh d1. d2, d3
    0x4f, 0x8b, 0x59, 0x45,     // fmls    q5, q10, q11
    0x6f, 0x9f, 0x21, 0x04,     // umlal   q4, hq8, d31
    0x0f, 0x88, 0x19, 0x84,     // fmla    d4, d12, q8
    0x4c, 0x00, 0x71, 0x04,     // st1     q4, [x8]
    0x0c, 0x00, 0x83, 0xe9,     // st2     d9, d10, [sp]
    0x4c, 0x00, 0xa3, 0xc2,     // st1     q2, q3, [x30]
    0x4c, 0x40, 0x63, 0xfe,     // ld1     q30, q31, q0, [sp]
    0x0c, 0x40, 0x20, 0x01,     // ld1     d1, d2, d3, d4, [x0]
    0x4c, 0x40, 0x40, 0x25,     // ld3     q5, q6, q7, [x1]
    0x0c, 0x94, 0x00, 0x40,     // st4     d0, d1, d2, d3, [x2], x20
    0x4c, 0x85, 0x23, 0xfe,     // st1     q30, q31, q0, q1, [sp], x5
    0x4c, 0x87, 0x61, 0x04,     // st1     q4, q5, q6, [x8], x7
    0x0c, 0xde, 0xa3, 0xff,     // ld1     d31, d0, [sp], x30
    0x4c, 0xc0, 0x71, 0x45,     // ld1     q5, [x10], x0
    0x0c, 0x9f, 0x43, 0xe5,     // st3     d5, d6, d7, [sp], #24
    0x4c, 0x9f, 0x20, 0x5f,     // st1     q31, q0, q1, q2, [x2], #48
    0x4c, 0xdf, 0x63, 0xe1,     // ld1     q1, q2, q3, [sp], #48
    0x0c, 0xdf, 0xa3, 0xdf,     // ld1     d31, d0, [x30], #16
    0x4c, 0x9f, 0x70, 0x04,     // st1     q4, [x0], #16
    0x4d, 0x00, 0x00, 0x45,     // st1     q5, [x2]
    0x0d, 0x20, 0x43, 0xe4,     // st2     d4, d5, [sp]
    0x4d, 0x00, 0xa1, 0x1f,     // st3     q31, q0, q1, [x8]
    0x0d, 0x20, 0xa7, 0xe0,     // st4     d0, d1, d2, d3, [sp]
    0x4d, 0x20, 0x43, 0xd5,     // st2     q21, q22, [x30]
    0x0d, 0x20, 0xa4, 0x1e,     // st4     d30, d31, d0, d1, [x0]
    0x4d, 0x40, 0x00, 0x45,     // ld1     q5, [x2]
    0x0d, 0x60, 0x43, 0xe4,     // ld2     d4, d5, [sp]
    0x4d, 0x40, 0xa1, 0x1f,     // ld3     q31, q0, q1, [x8]
    0x0d, 0x60, 0xa7, 0xe0,     // ld4     d0, d1, d2, d3, [sp]
    0x0d, 0x40, 0x80, 0x09,     // ld1     d9, [x0]
    0x4d, 0x40, 0xa4, 0x21,     // ld3     q1, q2, q3, [x1]
    0x4d, 0x88, 0x00, 0x45,     // st1     q5, [x2], x8
    0x0d, 0xbf, 0x43, 0xe4,     // st2     d4, d5, [sp], #4
    0x4d, 0x9e, 0xa1, 0x1f,     // st3     q31, q0, q1, [x8], x30
    0x0d, 0xbf, 0xa7, 0xe0,     // st4     d0, d1, d2, d3, [sp], #32
    0x4d, 0xdf, 0x00, 0x45,     // ld1     q5,[x2], #1
    0x0d, 0xe0, 0x43, 0xe4,     // ld2     d4, d5, [sp], x0
    0x4d, 0xdf, 0xa1, 0x1f,     // ld3     q31, q0, q1, [x8], #12
    0x0d, 0xe9, 0xa7, 0xe0,     // ld4     d0, d1, d2, d3, [sp], x9
    0x0f, 0x08, 0x04, 0x82,     // sshr    d2, d4, #8
    0x4f, 0x40, 0x14, 0x20,     // ssra    q0, q1, #64
    0x0f, 0x09, 0x57, 0xff,     // shl     d31, d31, #1
    0x4f, 0x30, 0x76, 0x08,     // sqshl   q8, q16, #16
    0x0f, 0x10, 0x9d, 0x02,     // sqrshrn d2, d8, #16
    0x4f, 0x20, 0xfc, 0x00,     // fcvtzs  q0, q0, #32
    0x2f, 0x28, 0x55, 0x04,     // sli     d4, d8, #8
    0x6f, 0x09, 0x94, 0xa2      // uqshrn  q2, q6, #7
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

  RegisterAST::Ptr xzr = create(aarch64::xzr);
  RegisterAST::Ptr wzr = create(aarch64::wzr);

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

  RegisterAST::Ptr hq0 = create(aarch64::hq0);
  RegisterAST::Ptr hq1 = create(aarch64::hq1);
  RegisterAST::Ptr hq2 = create(aarch64::hq2);
  RegisterAST::Ptr hq3 = create(aarch64::hq3);
  RegisterAST::Ptr hq4 = create(aarch64::hq4);
  RegisterAST::Ptr hq5 = create(aarch64::hq5);
  RegisterAST::Ptr hq6 = create(aarch64::hq6);
  RegisterAST::Ptr hq7 = create(aarch64::hq7);
  RegisterAST::Ptr hq8 = create(aarch64::hq8);
  RegisterAST::Ptr hq9 = create(aarch64::hq9);
  RegisterAST::Ptr hq10 = create(aarch64::hq10);
  RegisterAST::Ptr hq11 = create(aarch64::hq11);
  RegisterAST::Ptr hq12 = create(aarch64::hq12);
  RegisterAST::Ptr hq13 = create(aarch64::hq13);
  RegisterAST::Ptr hq14 = create(aarch64::hq14);
  RegisterAST::Ptr hq15 = create(aarch64::hq15);
  RegisterAST::Ptr hq16 = create(aarch64::hq16);
  RegisterAST::Ptr hq17 = create(aarch64::hq17);
  RegisterAST::Ptr hq18 = create(aarch64::hq18);
  RegisterAST::Ptr hq19 = create(aarch64::hq19);
  RegisterAST::Ptr hq20 = create(aarch64::hq20);
  RegisterAST::Ptr hq21 = create(aarch64::hq21);
  RegisterAST::Ptr hq22 = create(aarch64::hq22);
  RegisterAST::Ptr hq23 = create(aarch64::hq23);
  RegisterAST::Ptr hq24 = create(aarch64::hq24);
  RegisterAST::Ptr hq25 = create(aarch64::hq25);
  RegisterAST::Ptr hq26 = create(aarch64::hq26);
  RegisterAST::Ptr hq27 = create(aarch64::hq27);
  RegisterAST::Ptr hq28 = create(aarch64::hq28);
  RegisterAST::Ptr hq29 = create(aarch64::hq29);
  RegisterAST::Ptr hq30 = create(aarch64::hq30);
  RegisterAST::Ptr hq31 = create(aarch64::hq31);

  RegisterAST::Ptr sp(new RegisterAST(aarch64::sp));

  std::deque<registerSet> expectedRead, expectedWritten;

  // SADDLV H0, Q1.8B
  expectedRead.push_back({q1});
  expectedWritten.push_back({h0});

  // SMAXV B15, D8
  expectedRead.push_back({d8});
  expectedWritten.push_back({b15});

  // UADDLV S2, Q2
  expectedRead.push_back({q2});
  expectedWritten.push_back({s2});

  // UMINV B5, D0
  expectedRead.push_back({d0});
  expectedWritten.push_back({b5});

  // FMAXNMV S5, Q1
  expectedRead.push_back({q1});
  expectedWritten.push_back({s5});

  // DUP D1, Q2
  expectedRead.push_back({q2});
  expectedWritten.push_back({d1});

  // DUP Q20, W5
  expectedRead.push_back({w6});
  expectedWritten.push_back({q24});

  // DUP D0, X2
  expectedRead.push_back({x2});
  expectedWritten.push_back({d0});

  // INS Q4, X0
  expectedRead.push_back({x0});
  expectedWritten.push_back({q4});

  // INS Q31, WZR
  expectedRead.push_back({wzr});
  expectedWritten.push_back({q31});

  // SMOV W1, D5
  expectedRead.push_back({d5});
  expectedWritten.push_back({w1});

  // UMOV X2, Q2
  expectedRead.push_back({q2});
  expectedWritten.push_back({x2});

  // INS Q0, D0
  expectedRead.push_back({d0});
  expectedWritten.push_back({q0});

  // EXT D0, D1, D2, #5
  expectedRead.push_back({d1, d2});
  expectedWritten.push_back({d0});

  // EXT Q8, Q16, Q2, #15
  expectedRead.push_back({q16, q2});
  expectedWritten.push_back({q8});

  // MOVI D0, #8
  expectedRead.push_back({});
  expectedWritten.push_back({d0});

  // MOVI Q1, FF LSL #8
  expectedRead.push_back({});
  expectedWritten.push_back({q1});

  // ORR D5, #0 LSL #24
  expectedRead.push_back({d5});
  expectedWritten.push_back({d5});

  // ORR D5, #0 LSL #8
  expectedRead.push_back({d5});
  expectedWritten.push_back({d5});

  // FMOV Q8, FF
  expectedRead.push_back({});
  expectedWritten.push_back({q8});

  // MVNI Q0, #12 LSL #8
  expectedRead.push_back({});
  expectedWritten.push_back({q0});

  // BIC D8, #24 LSL #0
  expectedRead.push_back({q8});
  expectedWritten.push_back({q8});

  // MOVI D2, (all ones)
  expectedRead.push_back({});
  expectedWritten.push_back({d2});

  // UZP1 D8, D16, D0
  expectedRead.push_back({d16, d0});
  expectedWritten.push_back({d8});

  // TRN1 Q2, Q3, Q4
  expectedRead.push_back({q3, q4});
  expectedWritten.push_back({q2});

  // ZIP2 D8, D8, D8
  expectedRead.push_back({d8});
  expectedWritten.push_back({d8});

  // DUP B5, Q1
  expectedRead.push_back({q1});
  expectedWritten.push_back({b5});

  // DUP D8, D8
  expectedRead.push_back({d8});
  expectedWritten.push_back({d8});

  // ADDP D2, Q5
  expectedRead.push_back({q5});
  expectedWritten.push_back({d2});

  // FMAXNMP S1, D4
  expectedRead.push_back({d4});
  expectedWritten.push_back({s1});

  // FMINNMP D15, Q15
  expectedRead.push_back({q15});
  expectedWritten.push_back({d15});

  // SSHR D2, D4, #8
  expectedRead.push_back({d4});
  expectedWritten.push_back({d2});

  // SSRA D0, D1, #64
  expectedRead.push_back({d1});
  expectedWritten.push_back({d0});

  // SHL D31, D31, #1
  expectedRead.push_back({d31});
  expectedWritten.push_back({d31});

  // SQSHL B8, B16, 6
  expectedRead.push_back({b16});
  expectedWritten.push_back({b8});

  // SQRSHRN H2, H8, #16
  expectedRead.push_back({s8});
  expectedWritten.push_back({h2});

  // FCVTZS S0, S0, #32
  expectedRead.push_back({s0});
  expectedWritten.push_back({s0});

  // SLI D4, D8, #8
  expectedRead.push_back({d8});
  expectedWritten.push_back({d4});

  // UQSHRN B2, H2, #7
  expectedRead.push_back({h2});
  expectedWritten.push_back({b2});

  // SQDMLAL S8, H8, H0
  expectedRead.push_back({h8, h0});
  expectedWritten.push_back({s8});

  // SQDMLSL D2, S4, S1
  expectedRead.push_back({s4, s1});
  expectedWritten.push_back({d2});

  // SQDMULL S0, H0, H31
  expectedRead.push_back({h0, h31});
  expectedWritten.push_back({s0});

  // CMGT D2, D1, D0
  expectedRead.push_back({d1, d0});
  expectedWritten.push_back({d2});

  // SQSHL H0, H2, H4
  expectedRead.push_back({h2, h4});
  expectedWritten.push_back({h0});

  // SQDMULH S31, S1, S0
  expectedRead.push_back({s1, s0});
  expectedWritten.push_back({s31});

  // FRECPS D8, D8, D8
  expectedRead.push_back({d8});
  expectedWritten.push_back({d8});

  // URSHL D4, D31, D2
  expectedRead.push_back({d31, d2});
  expectedWritten.push_back({d4});

  // FCMGT S2, S8, S16
  expectedRead.push_back({s8, s16});
  expectedWritten.push_back({s2});

  // SUQADD B2, B8
  expectedRead.push_back({b8});
  expectedWritten.push_back({b2});

  // SQXTN H4, S0
  expectedRead.push_back({s0});
  expectedWritten.push_back({h4});

  // FCVTMS D31, D1
  expectedRead.push_back({d1});
  expectedWritten.push_back({d31});

  // FRECPE S9, S5
  expectedRead.push_back({s5});
  expectedWritten.push_back({s9});

  // SQXTUN B8, H2
  expectedRead.push_back({h2});
  expectedWritten.push_back({b8});

  // FCVTXN S15, D2
  expectedRead.push_back({d2});
  expectedWritten.push_back({s15});

  // FRSQRTE S31, S31
  expectedRead.push_back({s31});
  expectedWritten.push_back({s31});

  // SQDMLAL S2, H5, Q9
  expectedRead.push_back({s2, h5, q9});
  expectedWritten.push_back({s2});

  // SQDMLSL D0, S31, D31
  expectedRead.push_back({d0, d31, s31});
  expectedWritten.push_back({d0});

  // SQRDMULH H8, H4, D15
  expectedRead.push_back({h4, d15});
  expectedWritten.push_back({h8});

  // FMLA S9, S2, Q20
  expectedRead.push_back({s2, s9, q20});
  expectedWritten.push_back({s9});

  // FMLS D8, D5, D17
  expectedRead.push_back({d5, d8, d17});
  expectedWritten.push_back({d8});

  // TBL Q4, Q8, Q9
  expectedRead.push_back({q8, q9});
  expectedWritten.push_back({q4});

  // TBX D0, D1, D2, D3
  expectedRead.push_back({d1, d2, d3});
  expectedWritten.push_back({d0});

  // TBL Q0, Q24, Q25, Q26, Q31
  expectedRead.push_back({q24, q25, q26, q31});
  expectedWritten.push_back({q0});

  // TBX D20, D0, D1, D2, D3, D21
  expectedRead.push_back({d0, d1, d2, d3, d21});
  expectedWritten.push_back({d20});

  // SADDL Q1, HQ2, HQ3
  expectedRead.push_back({hq2, hq3});
  expectedWritten.push_back({q1});

  // SSUBL Q31, D4, D5
  expectedRead.push_back({d4, d5});
  expectedWritten.push_back({q31});

  // ADDHN HQ2, Q8, Q16
  expectedRead.push_back({q8, q16});
  expectedWritten.push_back({hq2});

  // SABDL Q5, D2, D8
  expectedRead.push_back({d2, d8});
  expectedWritten.push_back({q5});

  // SQDMLSL Q0, HQ1, HQ2
  expectedRead.push_back({hq1, hq2});
  expectedWritten.push_back({q0});

  // RSUBHN D2, Q15, Q31
  expectedRead.push_back({q15, q31});
  expectedWritten.push_back({d2});

  // SUBHN HQ8, Q2,Q3
  expectedRead.push_back({q2, q3});
  expectedWritten.push_back({hq8});

  // SQADD D2, D3, D5
  expectedRead.push_back({d3, d5});
  expectedWritten.push_back({d2});

  // CMGT Q0, Q1, Q31
  expectedRead.push_back({q1, q31});
  expectedWritten.push_back({q0});

  // SABA D8, D9, D10
  expectedRead.push_back({d9, d10});
  expectedWritten.push_back({d8});

  // BSL Q4, Q5, Q6
  expectedRead.push_back({q5, q6});
  expectedWritten.push_back({q4});

  // CNT D8, D9
  expectedRead.push_back({d9});
  expectedWritten.push_back({d8});

  // XTN Q0, Q31
  expectedRead.push_back({q31});
  expectedWritten.push_back({q0});

  // FCVTAS Q2, Q3
  expectedRead.push_back({q3});
  expectedWritten.push_back({q2});

  // FSQRT D31, D15
  expectedRead.push_back({d15});
  expectedWritten.push_back({d31});

  // SMLAL Q2, D5, D0
  expectedRead.push_back({d5, d0, q2});
  expectedWritten.push_back({q2});

  // SMLSL Q1, HQ31, Q5
  expectedRead.push_back({q1, hq31, q5});
  expectedWritten.push_back({q1});

  // MUL Q9, Q2, D31
  expectedRead.push_back({q2, d31});
  expectedWritten.push_back({q9});

  // SQDMULL Q8, D4, Q5
  expectedRead.push_back({d4, q5});
  expectedWritten.push_back({q8});

  // SQRDMULH D1. D2, D3
  expectedRead.push_back({d2, d3});
  expectedWritten.push_back({d1});

  // FMLS Q5, Q10, Q11
  expectedRead.push_back({q5, q10, q11});
  expectedWritten.push_back({q5});

  // UMLAL Q4, HQ8, D31
  expectedRead.push_back({q4, hq8, d31});
  expectedWritten.push_back({q4});

  // FMLA D4, D12, Q8
  expectedRead.push_back({d4, d12, q8});
  expectedWritten.push_back({d4});

  // ST1 Q4, [X8]
  expectedRead.push_back({q4, x8});
  expectedWritten.push_back({});

  // ST2 D9, D10, [SP]
  expectedRead.push_back({d9, d10, sp});
  expectedWritten.push_back({});

  // ST1 Q2, Q3, [X30]
  expectedRead.push_back({q2, q3, x30});
  expectedWritten.push_back({});

  // LD1 Q30, Q31, Q0, [SP]
  expectedRead.push_back({sp});
  expectedWritten.push_back({q0, q30, q31});

  // LD1 D1, D2, D3, D4, [X0]
  expectedRead.push_back({x0});
  expectedWritten.push_back({d1, d2, d3, d4});

  // LD3 Q5, Q6, Q7, [X1]
  expectedRead.push_back({x1});
  expectedWritten.push_back({q5, q6, q7});

  // ST4 D0, D1, D2, D3, [X2], X20
  expectedRead.push_back({x2, x20, d0, d1, d2, d3});
  expectedWritten.push_back({x2});

  // ST1 Q30, Q31, Q0, Q1, [SP], X5
  expectedRead.push_back({q30, q31, q0, q1, sp, x5});
  expectedWritten.push_back({sp});

  // ST1 Q4, Q5, Q6, [X8], X7
  expectedRead.push_back({q4, q5, q6, x7, x8});
  expectedWritten.push_back({x8});

  // LD1 D31, D0, [SP], X30
  expectedRead.push_back({sp, x30});
  expectedWritten.push_back({d0, d31, sp});

  // LD1 Q5, [X10], X0
  expectedRead.push_back({x10, x0});
  expectedWritten.push_back({q5, x10});

  // ST3 D5, D6, D7, [SP], #24
  expectedRead.push_back({d5, d6, d7, sp});
  expectedWritten.push_back({sp});

  // ST1 Q31, Q0, Q1, Q2, [X2], #48
  expectedRead.push_back({q0, q1, q2, q31, x2});
  expectedWritten.push_back({x2});

  // LD1 Q1, Q2, Q3, [SP], #48
  expectedRead.push_back({sp});
  expectedWritten.push_back({q1, q2, q3, sp});

  // LD1 D31, D0, [X30], #16
  expectedRead.push_back({x30});
  expectedWritten.push_back({d0, d31, x30});

  // ST1 Q4, [X0], #16
  expectedRead.push_back({q4, x0});
  expectedWritten.push_back({x0});

  // ST1 Q5, [X2]
  expectedRead.push_back({q5, x2});
  expectedWritten.push_back({});

  // ST2 D4, D5, [SP]
  expectedRead.push_back({d4, d5, sp});
  expectedWritten.push_back({});

  // ST3 Q31, Q0, Q1, [X8]
  expectedRead.push_back({q0, q1, q31, x8});
  expectedWritten.push_back({});

  // ST4 D0, D1, D2, D3, [SP]
  expectedRead.push_back({d0, d1, d2, d3, sp});
  expectedWritten.push_back({});

  // ST2 Q21, Q22, [X30]
  expectedRead.push_back({q21, q22, x30});
  expectedWritten.push_back({});

  // ST4 D30, D31, D0, D1, [X0]
  expectedRead.push_back({d0, d1, d30, d31, x0});
  expectedWritten.push_back({});

  // LD1 Q5, [X2]
  expectedRead.push_back({x2});
  expectedWritten.push_back({q5});

  // LD2 D4, D5, [SP]
  expectedRead.push_back({sp});
  expectedWritten.push_back({d4, d5});

  // LD3 Q31, Q0, Q1, [X8]
  expectedRead.push_back({x8});
  expectedWritten.push_back({q0, q1, q31});

  // LD4 D0, D1, D2, D3, [SP]
  expectedRead.push_back({sp});
  expectedWritten.push_back({d0, d1, d2, d3});

  // LD1 D9, [X0]
  expectedRead.push_back({x0});
  expectedWritten.push_back({d9});

  // LD3 Q1, Q2, Q3, [X1]
  expectedRead.push_back({x1});
  expectedWritten.push_back({q1, q2, q3});

  // ST1 Q5, [X2], X8
  expectedRead.push_back({q5, x2, x8});
  expectedWritten.push_back({x2});

  // ST2 D4, D5, [SP], #4
  expectedRead.push_back({d4, d5, sp});
  expectedWritten.push_back({sp});

  // ST3 Q31, Q0, Q1, [X8], X30
  expectedRead.push_back({q0, q1, q31, x8, x30});
  expectedWritten.push_back({x8});

  // ST4 D0, D1, D2, D3, [SP], #32
  expectedRead.push_back({d0, d1, d2, d3, sp});
  expectedWritten.push_back({sp});

  // LD1 Q5,[X2], #1
  expectedRead.push_back({x2});
  expectedWritten.push_back({q5, x2});

  // LD2 D4, D5, [SP], X0
  expectedRead.push_back({sp, x0});
  expectedWritten.push_back({sp, d4, d5});

  // LD3 Q31, Q0, Q1, [X8], #12
  expectedRead.push_back({x8});
  expectedWritten.push_back({q0, q1, q31, x8});

  // LD4 D0, D1, D2, D3, [SP], X9
  expectedRead.push_back({sp, x9});
  expectedWritten.push_back({d0, d1, d2, d3, sp});

  // SSHR D2, D4, #8
  expectedRead.push_back({d4});
  expectedWritten.push_back({d2});

  // SSRA Q0, Q1, #64
  expectedRead.push_back({q1});
  expectedWritten.push_back({q0});

  // SHL D31, D31, #1
  expectedRead.push_back({d31});
  expectedWritten.push_back({d31});

  // SQSHL Q8, Q16, #16
  expectedRead.push_back({q16});
  expectedWritten.push_back({q8});

  // SQRSHRN D2, D8, #16
  expectedRead.push_back({d8});
  expectedWritten.push_back({d2});

  // FCVTZS Q0, Q0, #32
  expectedRead.push_back({q0});
  expectedWritten.push_back({q0});

  // SLI D4, D8, #8
  expectedRead.push_back({d8});
  expectedWritten.push_back({d4});

  // UQSHRN Q2, Q6, #7
  expectedRead.push_back({q5});
  expectedWritten.push_back({q2});

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

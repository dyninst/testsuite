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

using namespace Dyninst;
using namespace InstructionAPI;

class aarch64_decode_Mutator : public InstructionMutator {
public:
  aarch64_decode_Mutator() {}

  virtual test_results_t executeTest() override;
};

extern "C" DLLEXPORT TestMutator* aarch64_decode_factory() {
  return new aarch64_decode_Mutator();
}

static void reverseBuffer(unsigned char* buffer, int bufferSize) {
  int elementCount = bufferSize / 4;

  for(int loop_index = 0; loop_index < elementCount; loop_index++) {
    std::swap(buffer[0], buffer[3]);
    std::swap(buffer[1], buffer[2]);
    buffer += 4;
  }
}

test_results_t aarch64_decode_Mutator::executeTest() {
  constexpr auto num_tests = 120;

  // clang-format off
  std::array<uint8_t, 4*num_tests> buffer = {
    0x0b, 0x0c, 0x01, 0x41,   // add w1, w10, w12
    0x0b, 0x08, 0x14, 0xa0,   // add w0, w5, w8, lsl #5
    0x8b, 0x49, 0x28, 0xe4,   // add x4, x7, x9, lsr #10
    0x4b, 0x04, 0x00, 0x40,   // sub w0, w2, w4
    0xcb, 0x8b, 0x1d, 0x06,   // sub x6, x8, x11, asr #7
    0x2b, 0x08, 0x14, 0xa0,   // adds w0, w5, w8, lsl #5
    0x0b, 0x2f, 0x69, 0x45,   // add w5, w10, w15
    0xcb, 0x21, 0x80, 0x20,   // sub x0, x1, x1, uxtb #0
    0xeb, 0x22, 0x70, 0x42,   // subs x2, x2, x2, sxtw #4
    0x1a, 0x19, 0x02, 0xc5,   // adc w5, w22, w25
    0xda, 0x02, 0x00, 0x20,   // sbc x0, x1, x2
    0x3a, 0x5e, 0x58, 0xeb,   // ccmn w7, #30, #11, 5
    0xfa, 0x48, 0xfa, 0x88,   // ccmp x20, #8, #0, 15
    0x3a, 0x4a, 0x10, 0xa7,   // ccmn w5, w10, #7, 1
    0xfa, 0x42, 0xa0, 0x84,   // ccmp x2, x4, #4, 10
    0x1a, 0x8f, 0x11, 0x45,   // csel w5, w10, w15, 1
    0x9a, 0x84, 0x54, 0x40,   // csinc x0, x2, x4, 5
    0xda, 0x96, 0x72, 0xb4,   // csinv x20, x21, x22, 7
    0x5a, 0x8a, 0xa4, 0xa1,   // csneg w1, w5, w9, 10
    0x5a, 0xc0, 0x00, 0x41,   // rbit w1, w2
    0xda, 0xc0, 0x0e, 0x8a,   // rev x10, x20
    0x5a, 0xc0, 0x13, 0xbe,   // clz w30, w29
    0xda, 0xc0, 0x15, 0x8b,   // cls x11, x12
    0x5a, 0xc0, 0x05, 0x80,   // rev16 w0, w12
    0x1a, 0xc4, 0x08, 0x40,   // udiv w0, w2, w4
    0x9a, 0xd9, 0x0e, 0x8f,   // sdiv x15, x20, x25
    0x1a, 0xcb, 0x21, 0x05,   // lslv w5, w8, w11
    0x9a, 0xcb, 0x29, 0x27,   // asrv x7, x9, x11
    0x1a, 0xc4, 0x2c, 0x10,   // rorv w16, w0, w4
    0x1b, 0x02, 0x00, 0x61,   // madd w1, w3, w2, w0
    0x9b, 0x10, 0xf9, 0x04,   // msub x4, x8, x16, x30
    0x9b, 0x21, 0x84, 0x00,   // smsubl x0, x0, x1, x1
    0x9b, 0xca, 0x28, 0xa5,   // umulh w5, w5, w10, w10
    0x0a, 0x03, 0x00, 0x41,   // and w1, w2, w3
    0x8a, 0x2a, 0x14, 0xa0,   // bic x0, x5, x10, lsl #5
    0x2a, 0x62, 0x28, 0x00,   // orn w0, w0, w2, lsr #10
    0xca, 0x96, 0x0a, 0xb4,   // eor x20, x21, x22, asr #2
    0xea, 0xe1, 0x20, 0x21,   // bics x1, x1, x1, ror #8
    0x11, 0x00, 0x2f, 0xe0,   // add w0, wsp, #11
    0x31, 0x40, 0x01, 0x45,   // adds w5, w10, #0, lsl #12
    0xd1, 0x40, 0x31, 0x5f,   // sub sp, x10, #12
    0x13, 0x19, 0x29, 0xcc,   // sbfm w12, w14, #25, #10
    0xb3, 0x40, 0x07, 0xc0,   // bfm x0, x30, #63, #0
    0xd3, 0x41, 0x20, 0x14,   // ubfm x20, x0, #8, #1
    0x13, 0x9e, 0x16, 0x8a,   // extr w10, w20, w30, #5
    0x93, 0xd0, 0xfd, 0x00,   // extr x0, x8, x16, #63
    0x12, 0x2a, 0x1f, 0x1f,   // and wsp, w24, #63
    0xb2, 0x00, 0x03, 0xdf,   // orr sp, x30, #0
    0xd2, 0x7f, 0xaf, 0x34,   // eor x20, x25, #
    0x72, 0x00, 0x25, 0x45,   // ands w5, w10, #9
    0x12, 0xa0, 0x02, 0xe4,   // movn w4, #23, lsl #1
    0xd2, 0xc0, 0x02, 0x54,   // movz x20, #18, lsl #2
    0xf2, 0xe0, 0x20, 0x01,   // movk x1, #256, lsl #3
    0x12, 0x80, 0x01, 0x08,   // movn w8, #8
    0x10, 0x80, 0x00, 0x00,   // adr x0, #
    0xf0, 0x00, 0x00, 0x3e,   // adrp x30, #7
    0x34, 0xff, 0xff, 0xef,   // cbz w15, #
    0xb5, 0x00, 0x00, 0x3e,   // cbnz x30, #1
    0x54, 0xff, 0xff, 0xe1,   // b.ne #
    0x54, 0x00, 0x07, 0xec,   // b.gt #63
    0x36, 0xf7, 0xff, 0xe4,   // tbz w4, #30, #
    0xb7, 0x80, 0x00, 0x19,   // tbnz x25, #0, #16
    0x37, 0x60, 0x01, 0x9f,   // tbnz wzr, #9, #12
    0x17, 0xff, 0xff, 0xff,   // b #
    0x94, 0x00, 0x00, 0x08,   // bl #8
    0xd6, 0x1f, 0x01, 0x80,   // br x12
    0xd6, 0x3f, 0x03, 0xc0,   // blr x30
    0xd6, 0x5f, 0x00, 0x00,   // ret x0
    0x1e, 0x3f, 0x20, 0x00,   // fcmp s0, s31
    0x1e, 0x30, 0x21, 0x08,   // fcmp d16, #0.0
    0x1e, 0x7f, 0x23, 0xc0,   // fcmp d31, d32
    0x1e, 0x3f, 0xa6, 0x88,   // fccmp s20, s31, #8, 10
    0x1e, 0x62, 0x04, 0x25,   // fccmp d1, d2, #5, 0
    0x1e, 0x6b, 0x55, 0x59,   // fccmpe d10, d1,, #9, 5
    0x1e, 0x23, 0x4c, 0x41,   // fcsel s1, s, s3, 4
    0x1e, 0x20, 0x41, 0x45,   // fmov s5, s10
    0x1e, 0x60, 0xc3, 0xff,   // fabs d30, d31
    0x1e, 0x64, 0xc0, 0x40,   // frintp d0, d2
    0x1e, 0xe2, 0x40, 0xa4,   // fcvt s4, h5
    0x1e, 0xe2, 0xc3, 0xe0,   // fcvt d0, h31
    0x1e, 0x22, 0xc0, 0x02,   // fcvt d2, s0
    0x1e, 0x63, 0xc3, 0xff,   // fcvt h31, d31
    0x1e, 0x62, 0x40, 0x21,   // fcvt s1, d1
    0x1e, 0x23, 0xc2, 0x08,   // fcvt h8, s16
    0x1e, 0x22, 0x08, 0x20,   // fmul s0, s1, s2
    0x1e, 0x7f, 0x3b, 0xdd,   // fsub d29, d30, d31
    0x1e, 0x6f, 0x19, 0x45,   // fdiv d5, d10, d15
    0x1e, 0x20, 0x4a, 0x08,   // fmax s8, s16, s0
    0x1e, 0x21, 0x78, 0x21,   // fninnm s1, s1, s1
    0x1f, 0x02, 0x0c, 0x20,   // fmadd s0, s1, s2, s3
    0x1f, 0x48, 0xc0, 0x82,   // fmsub d2, d4, d8, d16
    0x1f, 0x2b, 0x35, 0x6a,   // fnmadd s10, s11, s11, s13
    0x1f, 0x62, 0x84, 0x88,   // fnmsub d8, d4, d2, d1
    0x1e, 0x31, 0x10, 0x00,   // fmov s0, #88
    0x1e, 0x67, 0xf0, 0x1f,   // fmov d31, #7f
    0x1e, 0x02, 0x97, 0xc0,   // scvtf s0, w30, #59
    0x9e, 0x43, 0x24, 0x01,   // ucvtf d1, x0, #55
    0x1e, 0x02, 0xc1, 0x45,   // scvtf s5, w10, #64
    0x1e, 0x43, 0x84, 0x48,   // ucvtf d8, w2, #63
    0x1e, 0x19, 0xc4, 0x0b,   // fcvtzu w12, s0, #63
    0x9e, 0x58, 0xff, 0xfe,   // fcvtzs x30, d31, #0
    0x9e, 0x19, 0xe1, 0x41,   // fcvtzu x1, s10, #8
    0x1e, 0x58, 0xf1, 0x29,   // fcvtzs w9, d9, #4
    0x1e, 0x20, 0x00, 0xa8,   // fcvtns w8, s5
    0x1e, 0x27, 0x03, 0xc1,   // fmov s1, w30
    0xd4, 0x10, 0x00, 0x01,   // svc #32768
    0xd4, 0x00, 0x00, 0x03,   // smc #0
    0xd4, 0x40, 0x03, 0xc0,   // hlt #30
    0xd4, 0xa0, 0x00, 0x42,   // dcps2 #2
    0xd5, 0x03, 0x30, 0x5f,   // clrex
    0xd5, 0x03, 0x34, 0x9f,   // dsb #4
    0xd5, 0x03, 0x31, 0xbf,   // dmb #1
    0xd5, 0x03, 0x20, 0xbf,   // hint #5
    0xd5, 0x03, 0x45, 0xdf,   // msr 30, #5
    0xd5, 0x09, 0x23, 0x80,   // sys #1, #2, #3, #4, x0
    0xd5, 0x29, 0x23, 0x9e,   // sysl #1, #2, #3, #4, x30
    0xd5, 0x3b, 0x9c, 0xc1,   // mrs x1, pmceid0_el0
    0xd5, 0x3b, 0xe8, 0x40,   // mrs x0, pmevcntr2_el0
    0xd5, 0x1b, 0xe0, 0x21,   // msr cntpct_el0, x1
    0xd5, 0x1b, 0xef, 0xc0,   // msr pmevtyper30_el0, x0
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

  RegisterAST::Ptr pmceid0_el0 = create(aarch64::pmceid0_el0);
  RegisterAST::Ptr pmevcntr2_el0 = create(aarch64::pmevcntr2_el0);
  RegisterAST::Ptr cntpct_el0 = create(aarch64::cntpct_el0);
  RegisterAST::Ptr pmevtyper30_el0 = create(aarch64::pmevtyper30_el0);

  std::deque<registerSet> expectedRead, expectedWritten;

  // ADD W1, W10, W12
  expectedRead.push_back({w12, w10});
  expectedWritten.push_back({w1});

  // ADD W0, W5, W8, LSL #5
  expectedRead.push_back({w8, w5});
  expectedWritten.push_back({w0});

  // ADD X4, X7, X9, LSR #10
  expectedRead.push_back({x9, x7});
  expectedWritten.push_back({x4});

  // SUB W0, W2, W4
  expectedRead.push_back({w4, w2});
  expectedWritten.push_back({w0});

  // SUB X6, X8, X11, ASR #7
  expectedRead.push_back({x11, x8});
  expectedWritten.push_back({x6});

  // ADDS W0, W5, W8, LSL #5
  expectedRead.push_back({w8,w5});
  expectedWritten.push_back({w0,pstate});

  // ADD W5, W10, W15
  expectedRead.push_back({x15, w10});
  expectedWritten.push_back({w5});

  // SUB X0, X1, X1, UXTB #0
  expectedRead.push_back({w1, x1});
  expectedWritten.push_back({x0});

  // SUBS X2, X2, X2, SXTW #4
  expectedRead.push_back({x2,x2});
  expectedWritten.push_back({x2,pstate});

  // ADC W5, W22, W25
  expectedRead.push_back({w25, w22});
  expectedWritten.push_back({w5});

  // SBC X0, X1, X2
  expectedRead.push_back({x2, x1});
  expectedWritten.push_back({x0});

  // CCMN W7, #30, #11, 5
  expectedRead.push_back({w7,pstate});
  expectedWritten.push_back({pstate});

  // CCMP X20, #8, #0, 15
  expectedRead.push_back({x20,pstate});
  expectedWritten.push_back({pstate});

  // CCMN W5, W10, #7, 1
  expectedRead.push_back({w10,w5,pstate});
  expectedWritten.push_back({pstate});

  // CCMP X2, X4, #4, 10
  expectedRead.push_back({x2,x4,pstate});
  expectedWritten.push_back({pstate});

  // CSEL W5, W10, W15, 1
  expectedRead.push_back({w15,w10,pstate});
  expectedWritten.push_back({w5});

  // CSINC X0, X2, X4, 5
  expectedRead.push_back({x2,x4,pstate});
  expectedWritten.push_back({x0});

  // CSINV X20, X21, X22, 7
  expectedRead.push_back({x21,x22,pstate});
  expectedWritten.push_back({x20});

  // CSNEG W1, W5, W9, 10
  expectedRead.push_back({w5,w10,pstate});
  expectedWritten.push_back({w1});

  // RBIT W1, W2
  expectedRead.push_back({w2});
  expectedWritten.push_back({w1});

  // REV X10, X20
  expectedRead.push_back({x20});
  expectedWritten.push_back({x10});

  // CLZ W30, W29
  expectedRead.push_back({w29});
  expectedWritten.push_back({w30});

  // CLS X11, X12
  expectedRead.push_back({x12});
  expectedWritten.push_back({x11});

  // REV16 W0, W12
  expectedRead.push_back({w12});
  expectedWritten.push_back({w0});

  // UDIV W0, W2, W4
  expectedRead.push_back({w4, w2});
  expectedWritten.push_back({w0});

  // SDIV X15, X20, X25
  expectedRead.push_back({x25, x20});
  expectedWritten.push_back({x15});

  // LSLV W5, W8, W11
  expectedRead.push_back({w11, w8});
  expectedWritten.push_back({w5});

  // ASRV X7, X9, X11
  expectedRead.push_back({x11, x9});
  expectedWritten.push_back({x7});

  // RORV W16, W0, W4
  expectedRead.push_back({w4, w0});
  expectedWritten.push_back({w16});

  // MADD W1, W3, W2, W0
  expectedRead.push_back({w2, w0, w3});
  expectedWritten.push_back({w1});

  // MSUB X4, X8, X16, X30
  expectedRead.push_back({x16, x30, x8});
  expectedWritten.push_back({x4});

  // SMSUBL X0, X0, X1, X1
  expectedRead.push_back({x1, w1, w0});
  expectedWritten.push_back({x0});

  // UMULH W5, W5, W10, W10
  expectedRead.push_back({x10, x5});
  expectedWritten.push_back({x5});

  // AND W1, W2, W3
  expectedRead.push_back({w3, w2});
  expectedWritten.push_back({w1});

  // BIC X0, X5, X10, LSL #5
  expectedRead.push_back({x10, x5});
  expectedWritten.push_back({x0});

  // ORN W0, W0, W2, LSR #10
  expectedRead.push_back({w2, w0});
  expectedWritten.push_back({w0});

  // EOR X20, X21, X22, ASR #2
  expectedRead.push_back({x22, x21});
  expectedWritten.push_back({x20});

  // BICS X1, X1, X1, ROR #8
  expectedRead.push_back({x1, x1});
  expectedWritten.push_back({x1, pstate});

  // ADD W0, WSP, #11
  expectedRead.push_back({wsp});
  expectedWritten.push_back({w0});

  // ADDS W5, W10, #0, LSL #12
  expectedRead.push_back({w10});
  expectedWritten.push_back({w5, pstate});

  // SUB SP, X10, #12
  expectedRead.push_back({x10});
  expectedWritten.push_back({sp});

  // SBFM W12, W14, #25, #10
  expectedRead.push_back({w14});
  expectedWritten.push_back({w12});

  // BFM X0, X30, #63, #0
  expectedRead.push_back({x30});
  expectedWritten.push_back({x0});

  // UBFM X20, X0, #8, #1
  expectedRead.push_back({x0});
  expectedWritten.push_back({x20});

  // EXTR W10, W20, W30, #5
  expectedRead.push_back({w30, w20});
  expectedWritten.push_back({w10});

  // EXTR X0, X8, X16, #63
  expectedRead.push_back({x16, x8});
  expectedWritten.push_back({x0});

  // AND WSP, W24, #63
  expectedRead.push_back({w24});
  expectedWritten.push_back({wsp});

  // ORR SP, X30, #0
  expectedRead.push_back({x30});
  expectedWritten.push_back({sp});

  // EOR X20, X25, #
  expectedRead.push_back({x25});
  expectedWritten.push_back({x20});

  // ANDS W5, W10, #9
  expectedRead.push_back({w10});
  expectedWritten.push_back({w5, pstate});

  // MOVN W4, #23, LSL #1
  expectedRead.push_back({});
  expectedWritten.push_back({w4});

  // MOVZ X20, #18, LSL #2
  expectedRead.push_back({});
  expectedWritten.push_back({x20});

  // MOVK X1, #256, LSL #3
  expectedRead.push_back({});
  expectedWritten.push_back({x1});

  // MOVN W8, #8
  expectedRead.push_back({});
  expectedWritten.push_back({w8});

  // ADR X0, #
  expectedRead.push_back({pc});
  expectedWritten.push_back({x0});

  // ADRP X30, #7
  expectedRead.push_back({pc});
  expectedWritten.push_back({x30});

  // CBZ W15, #
  expectedRead.push_back({w15, pc});
  expectedWritten.push_back({pc});

  // CBNZ X30, #1
  expectedRead.push_back({x30, pc});
  expectedWritten.push_back({pc});

  // B.NE #
  expectedRead.push_back({pc, pstate});
  expectedWritten.push_back({pc});

  // B.GT #63
  expectedRead.push_back({pc, pstate});
  expectedWritten.push_back({pc});

  // TBZ W4, #30, #
  expectedRead.push_back({w4, pc});
  expectedWritten.push_back({pc});

  // TBNZ X25, #0, #16
  expectedRead.push_back({x25, pc});
  expectedWritten.push_back({pc});

  // TBNZ WZR, #9, #12
  expectedRead.push_back({wzr, pc});
  expectedWritten.push_back({pc});

  // B #
  expectedRead.push_back({pc});
  expectedWritten.push_back({pc});

  // BL #8
  expectedRead.push_back({pc});
  expectedWritten.push_back({pc});

  // BR X12
  expectedRead.push_back({x12});
  expectedWritten.push_back({pc});

  // BLR X30
  expectedRead.push_back({x30});
  expectedWritten.push_back({pc});

  // RET X0
  expectedRead.push_back({x0});
  expectedWritten.push_back({pc});

  // FCMP S0, S31
  expectedRead.push_back({s31, s0});
  expectedWritten.push_back({pstate});

  // FCMP D16, #0.0
  expectedRead.push_back({s8});
  expectedWritten.push_back({pstate});

  // FCMP D31, D32
  expectedRead.push_back({d31, d30});
  expectedWritten.push_back({pstate});

  // FCCMP S20, S31, #8, 10
  expectedRead.push_back({s31, s20, pstate});
  expectedWritten.push_back({pstate});

  // FCCMP D1, D2, #5, 0
  expectedRead.push_back({d2, d1, pstate});
  expectedWritten.push_back({pstate});

  // FCCMPE D10, D1,, #9, 5
  expectedRead.push_back({d11, d10, pstate});
  expectedWritten.push_back({pstate});

  // FCSEL S1, S, S3, 4
  expectedRead.push_back({s3, s2, pstate});
  expectedWritten.push_back({s1});

  // FMOV S5, S10
  expectedRead.push_back({s10});
  expectedWritten.push_back({s5});

  // FABS D30, D31
  expectedRead.push_back({d31});
  expectedWritten.push_back({d31});

  // FRINTP D0, D2
  expectedRead.push_back({d2});
  expectedWritten.push_back({d0});

  // FCVT S4, H5
  expectedRead.push_back({h5});
  expectedWritten.push_back({s4});

  // FCVT D0, H31
  expectedRead.push_back({h31});
  expectedWritten.push_back({d0});

  // FCVT D2, S0
  expectedRead.push_back({s0});
  expectedWritten.push_back({d2});

  // FCVT H31, D31
  expectedRead.push_back({d31});
  expectedWritten.push_back({h31});

  // FCVT S1, D1
  expectedRead.push_back({d1});
  expectedWritten.push_back({s1});

  // FCVT H8, S16
  expectedRead.push_back({s16});
  expectedWritten.push_back({h8});

  // FMUL S0, S1, S2
  expectedRead.push_back({s2, s1});
  expectedWritten.push_back({s0});

  // FSUB D29, D30, D31
  expectedRead.push_back({d31, d30});
  expectedWritten.push_back({d29});

  // FDIV D5, D10, D15
  expectedRead.push_back({d15, d10});
  expectedWritten.push_back({d5});

  // FMAX S8, S16, S0
  expectedRead.push_back({s0, s16});
  expectedWritten.push_back({s8});

  // FNINNM S1, S1, S1
  expectedRead.push_back({s1, s1});
  expectedWritten.push_back({s1});

  // FMADD S0, S1, S2, S3
  expectedRead.push_back({s2, s3, s1});
  expectedWritten.push_back({s0});

  // FMSUB D2, D4, D8, D16
  expectedRead.push_back({d8, d16, d4});
  expectedWritten.push_back({d2});

  // FNMADD S10, S11, S11, S13
  expectedRead.push_back({s11, s13, s11});
  expectedWritten.push_back({s10});

  // FNMSUB D8, D4, D2, D1
  expectedRead.push_back({d2, d1, d4});
  expectedWritten.push_back({d8});

  // FMOV S0, #88
  expectedRead.push_back({});
  expectedWritten.push_back({s0});

  // FMOV D31, #7F
  expectedRead.push_back({});
  expectedWritten.push_back({d31});

  // SCVTF S0, W30, #59
  expectedRead.push_back({w30});
  expectedWritten.push_back({s0});

  // UCVTF D1, X0, #55
  expectedRead.push_back({x0});
  expectedWritten.push_back({d1});

  // SCVTF S5, W10, #64
  expectedRead.push_back({w10});
  expectedWritten.push_back({s5});

  // UCVTF D8, W2, #63
  expectedRead.push_back({w2});
  expectedWritten.push_back({d8});

  // FCVTZU W12, S0, #63
  expectedRead.push_back({s0});
  expectedWritten.push_back({w11});

  // FCVTZS X30, D31, #0
  expectedRead.push_back({d31});
  expectedWritten.push_back({x30});

  // FCVTZU X1, S10, #8
  expectedRead.push_back({s10});
  expectedWritten.push_back({x1});

  // FCVTZS W9, D9, #4
  expectedRead.push_back({d9});
  expectedWritten.push_back({w9});

  // FCVTNS W8, S5
  expectedRead.push_back({s5});
  expectedWritten.push_back({w8});

  // FMOV S1, W30
  expectedRead.push_back({w30});
  expectedWritten.push_back({s1});

  // SVC #32768
  expectedRead.push_back({pstate});
  expectedWritten.push_back({});

  // SMC #0
  expectedRead.push_back({pstate});
  expectedWritten.push_back({});

  // HLT #30
  expectedRead.push_back({pstate});
  expectedWritten.push_back({});

  // DCPS2 #2
  expectedRead.push_back({pstate});
  expectedWritten.push_back({});

  // CLREX
  expectedRead.push_back({});
  expectedWritten.push_back({});

  // DSB #4
  expectedRead.push_back({});
  expectedWritten.push_back({});

  // DMB #1
  expectedRead.push_back({});
  expectedWritten.push_back({});

  // HINT #5
  expectedRead.push_back({});
  expectedWritten.push_back({});

  // MSR 30, #5
  expectedRead.push_back({});
  expectedWritten.push_back({pstate});

  // SYS #1, #2, #3, #4, X0
  expectedRead.push_back({x0});
  expectedWritten.push_back({});

  // SYSL #1, #2, #3, #4, X30
  expectedRead.push_back({});
  expectedWritten.push_back({x30});

  // MRS X1, PMCEID0_EL0
  expectedRead.push_back({pmceid0_el0});
  expectedWritten.push_back({x1});

  // MRS X0, PMEVCNTR2_EL0
  expectedRead.push_back({pmevcntr2_el0});
  expectedWritten.push_back({x0});

  // MSR CNTPCT_EL0, X1
  expectedRead.push_back({x1});
  expectedWritten.push_back({cntpct_el0});

  // MSR PMEVTYPER30_EL0, X0
  expectedRead.push_back({x0});
  expectedWritten.push_back({pmevtyper30_el0});

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

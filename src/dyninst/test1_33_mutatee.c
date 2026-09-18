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
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_33_func2(int x);
int test1_33_func3(int x);

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_33_globalVariable1 = -1;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

int global33_2 = 0;
/* External global so the store below can't be DCE'd (see func3 epilogue). */
int test1_33_func3_sink = 0;

int test1_33_mutatee() {
  int passed;

    /* The only possible failures occur in the mutator. */

    passed = test1_33_globalVariable1;
    if ( passed ) {
       logerror( "Passed test #33 (control flow graphs)\n" );
       test_passes(testname);
       return 0; /* Test passed */
    } else {
       logerror( "Failed test #33 (control flow graphs)\n" );
       return -1; /* Test failed */
    }

}

void test1_33_func2(int x) {
    /* dprintf("Hello\n"); */

    if (x == 1) {
        /* dprintf("Goodbye.\n"); */
	global33_2 = 1;
    } else {
        /* dprintf("See you.\n"); */
	global33_2 = 2;
    }

    /* dprintf("That's all.\n"); */
}

/* Force a jump table for func3's switch even when the mutatee TU is built -O0:
   modern gcc (>=12 at -O2, >=15 at every level) lowers a small dense switch to a
   decision tree, and at -O0 never emits a jump table at all, so this test (which
   checks Dyninst's jump-table CFG recovery) needs the switch compiled >= -O1.
   See dyninst/testsuite#278. clang tables it natively; the attribute drives gcc. */
__attribute__((optimize("O2")))
int test1_33_func3(int x) {
    /* dprintf("Entry.\n"); */

    /* The Intel compiler for IA-64 requires at least 18 entries to
       trigger the generation of a jump table. */
    switch (x) {
      case 0:
	/* dprintf("0\n"); */
	x += 11;
	break;
      case 1:
	/* dprintf("1\n"); */
	x += 10;
	break;
      case 2:
	/* dprintf("2\n"); */
	x-= 12;
	break;
      case 3:
	/* dprintf("3\n"); */
	x *= 33;
	break;
      case 4:
	/* dprintf("4\n"); */
	x /= 42;
	break;
      case 5:
	/* dprintf("5\n"); */
	x %= 57;
	break;
      case 6:
	/* dprintf("6\n"); */
	x <<= 2;
	break;
      case 7:
	/* dprintf("7\n"); */
	x >>= 3;
	break;
      case 8:
	/* dprintf("8\n"); */
	x ^= 0xfe;
	break;
      case 9:
	/* dprintf("9\n"); */
	x &= 0x44;
	break;
      case 10:
	/* dprintf("10\n"); */
	x |= 0x11;
	break;
      case 11:
	/* dprintf("11\n"); */
	x += 110;
	break;
      case 12:
	/* dprintf("12\n"); */
	x-= 112;
	break;
      case 13:
	/* dprintf("13\n"); */
	x *= 133;
	break;
      case 14:
	/* dprintf("14\n"); */
	x /= 142;
	break;
      case 15:
	/* dprintf("15\n"); */
	x %= 157;
	break;
      case 16:
	/* dprintf("16\n"); */
	x <<= 12;
	break;
      case 17:
	/* dprintf("17\n"); */
	x >>= 13;
	break;
      case 18:
	/* dprintf("18\n"); */
	x ^= 0x1fe;
	break;
      case 19:
	/* dprintf("19\n"); */
	x &= 0x144;
	break;
      case 20:
		x *= 17;
		break;
      case 21:
		x ^= 30;
		break;
      case 22:
		x |= 43;
		break;
      case 23:
		x &= 56;
		break;
      case 24:
		x += 69;
		break;
      case 25:
		x -= 82;
		break;
      case 26:
		x *= 95;
		break;
      case 27:
		x ^= 108;
		break;
      case 28:
		x |= 121;
		break;
      case 29:
		x &= 134;
		break;
      case 30:
		x += 147;
		break;
      case 31:
		x -= 160;
		break;
      case 32:
		x *= 173;
		break;
      case 33:
		x ^= 186;
		break;
      case 34:
		x |= 199;
		break;
      case 35:
		x &= 212;
		break;
      case 36:
		x += 225;
		break;
      case 37:
		x -= 238;
		break;
      case 38:
		x *= 251;
		break;
      case 39:
		x ^= 13;
		break;
      case 40:
		x |= 26;
		break;
      case 41:
		x &= 39;
		break;
      case 42:
		x += 52;
		break;
      case 43:
		x -= 65;
		break;
      case 44:
		x *= 78;
		break;
      case 45:
		x ^= 91;
		break;
      case 46:
		x |= 104;
		break;
      case 47:
		x &= 117;
		break;
      case 48:
		x += 130;
		break;
      case 49:
		x -= 143;
		break;
      case 50:
		x *= 156;
		break;
      case 51:
		x ^= 169;
		break;
      case 52:
		x |= 182;
		break;
      case 53:
		x &= 195;
		break;
      case 54:
		x += 208;
		break;
      case 55:
		x -= 221;
		break;
      case 56:
		x *= 234;
		break;
      case 57:
		x ^= 247;
		break;
      case 58:
		x |= 9;
		break;
      case 59:
		x &= 22;
		break;
      case 60:
		x += 35;
		break;
      case 61:
		x -= 48;
		break;
      case 62:
		x *= 61;
		break;
      case 63:
		x ^= 74;
		break;
    };

    /* dprintf("Exit.\n"); */

    /* Anchor a single high-in-degree merge block so the switch's convergence
       survives -O2 tail-duplication (foundSwitchIn), paired with the jump-table
       dispatch (foundSwitchOut). See dyninst/testsuite#278. */
    test1_33_func3_sink += x;

    return x;
}

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
#include <string.h>

void test_reloc_func1();
void test_reloc_func2();

static void test_reloc_foo(int);
static void test_reloc_bar(int);

static int global_foo_val = 0;

void test_reloc_func1() {
    dprintf("In test_reloc_func1. About to call test_reloc_foo.\n");
    int len = strlen(testname);
    test_reloc_foo(len);
}

static void test_reloc_foo(int arg) {
    int ret = 1, idx;
    for(idx = 0; idx < arg; idx++)
        ret += idx;
    global_foo_val = ret;
    dprintf("In test_reloc_foo. Value of local computation is %d.\n", ret);
}

void test_reloc_func2() {
    dprintf("In test_reloc_func2. About to call test_reloc_bar.\n");
    const char *funcname = __func__;
    int len = strlen(funcname);
    test_reloc_bar(len);
}

static void test_reloc_bar(int arg) {
    int ret = 1, idx;
    for(idx = arg; idx >= 0; idx--)
        ret -= idx;
    dprintf("In test_reloc_bar. Value of local computation is %d.\n", ret);
}

int test_reloc_mutatee() {
    int ret = 1, idx;
    for(idx = 0; idx < strlen(testname); idx++)
        ret += idx;

    test_reloc_foo(strlen(testname));
    if(ret == global_foo_val) {
        test_passes(testname);
        return 0;
    } else {
        return -1;
    }
}

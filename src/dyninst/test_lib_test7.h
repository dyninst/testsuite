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
#include <test_lib_dll.h>
#include "BPatch.h"
#include "BPatch_thread.h"

/* Test7 Definitions */
typedef enum { Parent_p, Child_p } procType;
typedef enum { PreFork, PostFork } forkWhen;

struct  msgSt {
  long  mtype;     /* message type */
  char  mtext[1];  /* message text */
};
typedef struct msgSt ipcMsg;
typedef struct msgSt ipcMsg;

/* Test7 Functions */
TESTLIB_DLL_EXPORT bool setupMessaging(int *msgid);
TESTLIB_DLL_EXPORT bool doError(bool *passedTest, bool cond, const char *str);
TESTLIB_DLL_EXPORT bool verifyProcMemory(BPatch_process *appThread, const char *name,
                      int expectedVal, procType proc_type);
TESTLIB_DLL_EXPORT bool verifyProcMemory(const char *name, BPatch_variableExpr *var,
                      int expectedVal, procType proc_type);
TESTLIB_DLL_EXPORT void showFinalResults(bool passedTest, int i);


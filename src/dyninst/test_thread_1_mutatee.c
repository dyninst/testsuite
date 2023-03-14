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
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"
#include "test_thread.h"
#include "test12.h"
#include "dyninstRTExport.h"
#include "atomic.h"

/********************************************************************
Subtest 1:  rtlib spinlocks
*
*  This tests the dyninst_lock_t implementation by modifying an
*  atomic value while holding a lock.
*
********************************************************************/

void (*DYNINSTinit_thelock)(dyninst_lock_t *);
int (*DYNINSTlock_thelock)(dyninst_lock_t *);
void (*DYNINSTunlock_thelock)(dyninst_lock_t *);

static dyninst_lock_t test1lock;

Thread_t test1threads[TEST1_THREADS];
int subtest1err = 0;
testsuite_atomic(pthread_t, canary, 0)

/*
 * This barrier is a (maybe) temporary and incomplete fix for the threading
 * issues in proccontrol's mailbox. It mitigates the bug there by ensuring
 * that no thread in this mutatee can finish execution before all of the
 * other threads are created.
 */
pthread_barrier_t startup_barrier;

void *thread_main1 (void *arg)
{
   arg = NULL; /*Silence warnings*/

   pthread_barrier_wait(&startup_barrier);

   // We need a unique value for each thread, so just use its pthread ID
   pthread_t id = pthread_self();

   (*DYNINSTlock_thelock)(&test1lock);

   // This is atomic
   canary = id;

   if(canary != id) subtest1err = 1;

   canary = id;

   (*DYNINSTunlock_thelock)(&test1lock);

   return NULL;
}

int func1_1()
{
#if defined(m32_test)
  const char * libname = "libdyninstAPI_RT_m32.so";
#else
  const char *libname = "libdyninstAPI_RT.so";
#endif

  void *RTlib = dlopen(libname, RTLD_NOW);
  if (!RTlib) {
    logerror("%s[%d]:  could not open dyninst RT lib: %s\n", __FILE__, __LINE__, dlerror());
    char *ld = getenv("LD_LIBRARY_PATH");
    logerror("%s[%d]:  with LD_LIBRARY_PATH of: %s\n", __FILE__, __LINE__, (ld ? ld : "<NULL>"));
    return -1;
  }

  DYNINSTinit_thelock = (void (*)(dyninst_lock_t *))dlsym(RTlib, "dyninst_init_lock");
  if (!DYNINSTinit_thelock) {
    logerror("%s[%d]:  could not DYNINSTinit_thelock: %s\n", __FILE__, __LINE__, dlerror());
    return -1;
  }

  DYNINSTlock_thelock = (int (*)(dyninst_lock_t *))dlsym(RTlib, "dyninst_lock");
  if (!DYNINSTlock_thelock) {
    logerror("%s[%d]:  could not DYNINSTlock_thelock: %s\n", __FILE__, __LINE__, dlerror());
    return -1;
  }

  DYNINSTunlock_thelock = (void (*)(dyninst_lock_t *))dlsym(RTlib, "dyninst_unlock");
  if (!DYNINSTunlock_thelock) {
    logerror("%s[%d]:  could not DYNINSTunlock_thelock:%s\n", __FILE__, __LINE__, dlerror());
    return -1;
  }

  pthread_barrier_init(&startup_barrier, NULL, TEST1_THREADS);
  (*DYNINSTinit_thelock)(&test1lock);

  (*DYNINSTlock_thelock)(&test1lock);
  createThreads(TEST1_THREADS, thread_main1, test1threads);

  dprintf("%s[%d]:  doing initial unlock...\n", __FILE__, __LINE__);
  (*DYNINSTunlock_thelock)(&test1lock);

  for(int i=0; i<TEST1_THREADS; i++) {
	pthread_join(test1threads[i], NULL);
  }

  dlclose(RTlib);

  pthread_mutex_destroy(&real_lock);
  pthread_barrier_destroy(&startup_barrier);

  return subtest1err;
}

/* skeleton test doesn't do anything besides say that it passed */
int test_thread_1_mutatee() {
  if (func1_1() != 0) {
    return -1; /* Error of some kind */
  }

  test_passes(testname);
  return 0;
}

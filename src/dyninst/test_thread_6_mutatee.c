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
#include "atomic.h"

// This is modified by the mutator
volatile int proc_current_state = 0;

/*
 * Flag to tell the created threads to stop running
 *
 * Because this is only written to from the main thread,
 * we don't need strong memory consistency. Making it 'volatile'
 * is enough to prevent the compiler from removing the check
 * in 'init_func'
*/
testsuite_atomic(int, done, 0)

// Barrier to synchronize thread startup
testbarrier_t startup_barrier;

// This must have external linkage so that the mutator can find the symbol
void* init_func(void *arg)
{
	waitTestBarrier(&startup_barrier);
	while(!done);
	return arg;
}

int test_thread_6_mutatee() {
	const int NUM_THREADS = 4;

	// We need all of the threads _and_ the main thread to wait on the barrier
	initBarrier(&startup_barrier, NUM_THREADS + 1);
	initThreads();

	thread_t thread_ids[NUM_THREADS];

	for (int i=0; i<NUM_THREADS; i++) {
		thread_ids[i] = spawnNewThread((void*)init_func, NULL);
	}

	// Make sure all of the threads have spooled up before proceeding
	waitTestBarrier(&startup_barrier);

	// Wait for mutator to attach (if in attach mode)
	handleAttach();

	logstatus("[%s:%d]: stage 1 - all threads created\n", __FILE__, __LINE__);

	// Wait until mutator has modified our state
	while(proc_current_state == 0);

	logstatus("[%s:%d]: stage 2 - State changed by mutator; threads exiting\n", __FILE__, __LINE__);

	// Flag all the threads to complete
	done = 1;

	for(int i=0; i<NUM_THREADS; i++) {
		joinThread(thread_ids[i]);
	}

	logstatus("[%s:%d]: stage 3 - all threads joined\n", __FILE__, __LINE__);

	testBarrierDestroy(&startup_barrier);

	logstatus("[%s:%d]: stage 4 - synchronization cleanup complete\n", __FILE__, __LINE__);

	return 0;
}

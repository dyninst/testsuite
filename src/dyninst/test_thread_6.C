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

#include "dyninst_comp.h"
#include "test_lib.h"
#include <BPatch.h>
#include <BPatch_function.h>
#include <BPatch_process.h>
#include <BPatch_thread.h>
#include <atomic>
#include <cstdio>
#include <unordered_map>

class test_thread_6_Mutator : public DyninstMutator {
protected:
  BPatch *bpatch;

  void upgrade_mutatee_state();
  BPatch_process *getProcess();
  test_results_t mutatorTest(BPatch *bpatch);

public:
  bool hasCustomExecutionPath() override { return true; }
  test_results_t setup(ParameterDict &param) override;
  test_results_t executeTest() override;
};
extern "C" DLLEXPORT TestMutator *test_thread_6_factory() {
  return new test_thread_6_Mutator();
}

namespace {
  BPatch_process *mutatee_process;
  std::atomic<unsigned> thread_count;

  // Map the BPatchID to the tid
  std::unordered_map<unsigned, dynthread_t> tids;
  std::mutex tids_mtx;

  std::atomic<unsigned> deleted_threads;

  std::atomic<unsigned> error13;

  bool debug_flag = false;
  std::mutex print_mtx;

  constexpr auto NUM_THREADS = 5;
  constexpr auto TIMEOUT = 20;
}

template <typename... Args> static void dprintf(char const *fmt, Args... args) {
  if (debug_flag) {
    std::lock_guard<std::mutex> l{print_mtx};
    fprintf(stdout, fmt, args...);
    fflush(stdout);
  }
}

template <typename Container, typename Value>
bool exists(Container const &c, std::mutex &m, Value v) {
  std::lock_guard<std::mutex> l{m};
  return c.count(v) > 0U;
}
template <typename Container, typename Value>
void remove(Container &c, std::mutex &m, Value v) {
  std::lock_guard<std::mutex> l{m};
  c.erase(v);
}
template <typename Container, typename Key, typename Value>
void insert(Container &c, std::mutex &m, Key k, Value v) {
  std::lock_guard<std::mutex> l{m};
  c[k] = v;
}
template <typename Container> void clear(Container &c, std::mutex &m) {
  std::lock_guard<std::mutex> l{m};
  c.clear();
}
template <typename Container, typename Value>
bool has_value(Container const &c, std::mutex &m, Value v) {
  std::lock_guard<std::mutex> l{m};
  for (auto const &p : tids) {
    if (p.second == v) {
      return true;
    }
  }
  return false;
}

static void deadthr(BPatch_process *my_proc, BPatch_thread *thr) {
  dprintf("%s[%d]:  welcome to deadthr\n", __FILE__, __LINE__);

  if (!thr) {
    dprintf("%s[%d]:  deadthr called without valid ptr to thr\n", __FILE__,
            __LINE__);
  }

  const auto thr_bp_id = thr->getBPatchID();
  if (!exists(tids, tids_mtx, thr_bp_id)) {
    dprintf("%s[%d]:  deadthr called on unknown thread %u\n", __FILE__,
            __LINE__, thr_bp_id);
  }

  if (my_proc != mutatee_process) {
    dprintf("[%s:%u] - Got invalid process: %p vs %p\n", __FILE__, __LINE__,
            my_proc, mutatee_process);
    error13.store(1);
  }
  remove(tids, tids_mtx, thr_bp_id);
  deleted_threads++;
  dprintf("%s[%d]:  leaving to deadthr, %d is dead, %d total dead threads\n",
          __FILE__, __LINE__, thr_bp_id, deleted_threads.load());
}

static void newthr(BPatch_process *my_proc, BPatch_thread *thr) {
  dprintf("%s[%d]:  welcome to newthr, error13 = %d\n", __FILE__, __LINE__,
          error13.load());

  if (my_proc != mutatee_process) {
    dprintf("[%s:%u] - Got invalid process: %p vs %p\n", __FILE__, __LINE__,
            my_proc, mutatee_process);
    error13.store(1);
  }

  if (thr->isDeadOnArrival()) {
    dprintf("[%s:%u] - Got a dead on arival thread\n", __FILE__, __LINE__);
    error13.store(1);
  }

  const auto thr_bp_id = thr->getBPatchID();
  dprintf("%s[%d]:  newthr: BPatchID = %u\n", __FILE__, __LINE__, thr_bp_id);

  const auto mytid = thr->getTid();
  dprintf("%s[%d]:  newthr: tid = %lu\n", __FILE__, __LINE__,
          static_cast<unsigned long>(mytid));

  // Each thread should only cause the callback to be invoked once
  if (exists(tids, tids_mtx, thr_bp_id)) {
    dprintf("[%s:%d] - WARNING: Thread %u called in callback twice\n", __FILE__,
            __LINE__, thr_bp_id);
    error13.store(1);
  }

  if (has_value(tids, tids_mtx, mytid)) {
    dprintf("[%s:%d] - WARNING: Thread %u has a duplicate tid (%d)\n", __FILE__,
            __LINE__, thr_bp_id, static_cast<int>(mytid));
    error13.store(1);
  }

  insert(tids, tids_mtx, thr_bp_id, mytid);
  thread_count++;
}

void test_thread_6_Mutator::upgrade_mutatee_state() {
  dprintf("%s[%d]:  welcome to upgrade_mutatee_state\n", __FILE__, __LINE__);
  BPatch_variableExpr *var;
  BPatch_image *img = mutatee_process->getImage();
  var = img->findVariable("proc_current_state");
  dprintf("%s[%d]: upgrade_mutatee_state: stopping for read...\n", __FILE__,
          __LINE__);
  mutatee_process->stopExecution();
  int val = 0;
  var->readValue(&val);
  val++;
  var->writeValue(&val);
  mutatee_process->continueExecution();
  dprintf("%s[%d]:  upgrade_mutatee_state: continued after write, val = %d\n",
          __FILE__, __LINE__, val);
}

BPatch_process *test_thread_6_Mutator::getProcess() { return appProc; }

static void register_mutator_threads(BPatch_process *appProc) {
  std::vector<BPatch_thread *> threads;
  appProc->getThreads(threads);
  dprintf("Found %zu mutator threads\n", threads.size());
  for (auto *t : threads) {
    newthr(appProc, t);
  }
}

static bool wait_mutatee_threads(BPatch *bpatch) {
  unsigned num_attempts = 0;
  // Wait for NUM_THREADS new thread callbacks to run
  while (thread_count.load() < NUM_THREADS) {
    dprintf("Going into waitForStatusChange...\n");
    bpatch->waitForStatusChange();
    dprintf("Back from waitForStatusChange...\n");
    if (mutatee_process->isTerminated()) {
      dprintf("[%s:%d] - App exited early\n", __FILE__, __LINE__);
      error13.store(1);
      break;
    }
    if (num_attempts++ == TIMEOUT) {
      dprintf("[%s:%d] - Timed out waiting for mutatee threads\n", __FILE__,
              __LINE__);
      dprintf("[%s:%d] - Only have %u threads, expected %u!\n", __FILE__,
              __LINE__, thread_count.load(), NUM_THREADS);
      return false;
    }
    P_sleep(1);
  }

  dprintf("%s[%d]:  done waiting for thread creations, error13 = %d\n",
          __FILE__, __LINE__, error13.load());

  return true;
}

static bool check_mutatee_thread_count() {
  BPatch_Vector<BPatch_thread *> thrds;
  mutatee_process->getThreads(thrds);
  dprintf("Found %zu mutatee threads\n", thrds.size());

  if (thrds.size() != NUM_THREADS) {
    dprintf("Have %u mutatee threads, expected %u!\n", thrds.size(),
            NUM_THREADS);
    return false;
  }
  return true;
}

static bool wait_mutatee_completion(BPatch *bpatch) {
  dprintf("Waiting for mutatee to complete.\n");

  auto num_attempts = 0;
  while (!mutatee_process->isTerminated()) {
    mutatee_process->continueExecution();
    bpatch->waitForStatusChange();
    if (++num_attempts >= TIMEOUT) {
      dprintf("Timed out waiting for mutatee to complete\n");
      return false;
    }
  }
  dprintf("Mutatee is complete.\n");
  return true;
}

static bool check_deleted_thread_count() {
  const auto cnt = deleted_threads.load();
  if (cnt != 0U) {
    dprintf("ERROR: %zu threads terminated early\n", cnt);
    return false;
  }
  return true;
}

static bool wait_thread_termination() {
  auto num_attempts = 0;
  do {
    const auto cnt = deleted_threads.load();
    if (cnt == NUM_THREADS)
      return true;
    if (++num_attempts >= TIMEOUT) {
      dprintf("Detecting deleted threads timed out: Got %u, but expected %d\n",
              cnt, NUM_THREADS);
      return false;
    }
    P_sleep(1);
  } while (1);
  return true;
}

test_results_t test_thread_6_Mutator::mutatorTest(BPatch *bpatch) {
  mutatee_process->continueExecution();

  // Register callbacks for threads in this process
  register_mutator_threads(this->appProc);

  auto terminate_mutatee = [this] {
    // Be sure to have the threads in the mutatee resume so they aren't
    // blocking on the barrier
    upgrade_mutatee_state();

    // Terminate the mutatee
    if (!mutatee_process->isTerminated())
      mutatee_process->terminateExecution();
  };

  // Wait for the mutatee threads to invoke their callbacks
  if (!wait_mutatee_threads(this->bpatch)) {
    terminate_mutatee();
    return FAILED;
  }

  // Make sure we got the expected number of threads
  if (!check_mutatee_thread_count()) {
    terminate_mutatee();
    return FAILED;
  }

  // Check for errors encountered by the callbacks
  if (error13.load()) {
    dprintf("ERROR during thread create stage, exiting\n");
    dprintf("*** Failed test_thread_6 (Threading Callbacks)\n");
    terminate_mutatee();
    return FAILED;
  }

  // All threads should be alive here
  if (!check_deleted_thread_count()) {
    terminate_mutatee();
    return FAILED;
  }

  // If the intialization was successful, change the mutatee state
  // to allow its threads to complete
  upgrade_mutatee_state();

  // Wait for the mutatee to complete
  if (!wait_mutatee_completion(this->bpatch)) {
    terminate_mutatee();
    return FAILED;
  }

  // Wait for all threads (except this one) to terminate
  if (!wait_thread_termination()) {
    // Print the threads not deleted
    // NOTE: It is possible there are no threads here because they
    //	   terminated between the check in
    //	   `wait_thread_termination` and locking the mutex here
    std::lock_guard<std::mutex> l{tids_mtx};
    for (auto const &p : tids) {
      dprintf("Thread %u:%d wasn't deleted\n", p.first,
              static_cast<int>(p.second));
    }

    terminate_mutatee();
    return FAILED;
  }

  // Check for errors encountered by the callbacks
  if (error13.load()) {
    dprintf("ERROR during thread termination stage, exiting\n");
    dprintf("*** Failed test_thread_6 (Threading Callbacks)\n");
    terminate_mutatee();
    return FAILED;
  }

  dprintf("Passed test_thread_6 (Threading Callbacks)\n");
  dprintf("Test completed without errors\n");
  return PASSED;
}

test_results_t test_thread_6_Mutator::executeTest() {
  BPatch_process *appProc = appThread->getProcess();
  if (appProc && !appProc->supportsUserThreadEvents()) {
    dprintf("System does not support user thread events\n");
    appThread->getProcess()->terminateExecution();
    return SKIPPED;
  }

  mutatee_process = getProcess();
  if (!mutatee_process)
    return FAILED;

  thread_count.store(0U);
  deleted_threads.store(0U);
  error13.store(0U);
  clear(tids, tids_mtx);

  test_results_t rv = mutatorTest(bpatch);

  if (!bpatch->removeThreadEventCallback(BPatch_threadCreateEvent, newthr) ||
      !bpatch->removeThreadEventCallback(BPatch_threadDestroyEvent, deadthr)) {
    dprintf("%s[%d]:  failed to remove thread callback\n", __FILE__, __LINE__);
    return FAILED;
  }

  return rv;
}

test_results_t test_thread_6_Mutator::setup(ParameterDict &param) {
  bpatch = (BPatch *)(param["bpatch"]->getPtr());

  if (param["debugPrint"]->getInt() != 0) {
    debug_flag = true;
  }

  if (!bpatch->registerThreadEventCallback(BPatch_threadCreateEvent, newthr) ||
      !bpatch->registerThreadEventCallback(BPatch_threadDestroyEvent,
                                           deadthr)) {
    dprintf("%s[%d]:  failed to register thread callback\n", __FILE__,
            __LINE__);
    return FAILED;
  }

  appProc = (BPatch_process *)(param["appProcess"]->getPtr());
  if (appProc)
    appImage = appProc->getImage();

  return DyninstMutator::setup(param);
}

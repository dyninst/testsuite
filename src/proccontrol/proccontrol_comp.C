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
#include "ParameterDict.h"
#include "proccontrol_comp.h"
#include "communication.h"
#include "MutateeStart.h"
#include "SymReader.h"
#include "PCErrors.h"
#include "PlatFeatures.h"

#include <cstdio>
#include <cerrno>
#include <cstring>

#include <set>
#include <vector>
#include <map>

using namespace std;

#if !defined(os_windows_test)

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>

#else

#include <process.h>
#include <winsock2.h>
#if !defined(MSG_WAITALL)
#define MSG_WAITALL 8
#endif
#define MSG_NOSIGNAL 0 // override unix-ism

#endif

#if !defined(os_windows_test)
typedef int SOCKET;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

static int closesocket(int sock) {
   return close(sock);
}

struct socket_types
{
	typedef sockaddr_un sockaddr_t;
	static SOCKET socket()
	{
		return ::socket(AF_UNIX, SOCK_STREAM, 0);
	}
	static sockaddr_t make_addr()
	{
	   sockaddr_t addr;
	   memset(&addr, 0, sizeof(socket_types::sockaddr_t));
	   addr.sun_family = AF_UNIX;
	   snprintf(addr.sun_path, sizeof(addr.sun_path)-1, "/tmp/pct%d", getpid());
	   return addr;
	}
	static bool recv(unsigned char *msg, unsigned msg_size, int sfd, int notification_fd)
	{
	   int result;
	   for (;;) {
		  int nfds = sfd > notification_fd ? sfd : notification_fd;
		  nfds++;
		  fd_set readset; FD_ZERO(&readset);
		  fd_set writeset; FD_ZERO(&writeset);
		  fd_set exceptset; FD_ZERO(&exceptset);
		  FD_SET(sfd, &readset);
		  FD_SET(notification_fd, &readset);
		  struct timeval timeout;
		  timeout.tv_sec = RECV_TIMEOUT;
		  timeout.tv_usec = 0;
		  do {
			 result = select(nfds, &readset, &writeset, &exceptset, &timeout);
		  } while (result == -1 && errno == EINTR);

		  if (result == 0) {
			 logerror("Timeout while waiting for communication\n");
			 return false;
		  }
		  if (result == -1) {
			 char error_str[1024];
			 snprintf(error_str, 1024, "Error calling select: %s\n", strerror(errno));
			 logerror(error_str);
			 return false;
		  }

		  if (FD_ISSET(notification_fd, &readset)) {
			 bool result = Process::handleEvents(true);
			 if (!result) {
				logerror("Failed to handle process events\n");
				return false;
			 }
		  }
		  if (FD_ISSET(sfd, &readset)) {
			 break;
		  }
	   }

	   result = ::recv(sfd, (char *)(msg), msg_size, MSG_WAITALL);
	   if (result == -1) {
		  char error_str[1024];
		  snprintf(error_str, 1024, "Unable to recieve message: %s\n", strerror(errno));
		  logerror(error_str);
		  return false;
	   }
	   return true;
	}

	static int close(SOCKET s)
	{
		return ::close(s);
	}
	typedef ::socklen_t socklen_t;
};

#else

struct socket_types
{
	typedef sockaddr_in sockaddr_t;
	static SOCKET socket()
	{
		return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	static sockaddr_t make_addr()
	{
	   sockaddr_t addr;
	   memset(&addr, 0, sizeof(socket_types::sockaddr_t));
	   addr.sin_family = AF_INET;
	   addr.sin_port = htons((int) GetCurrentProcessId()); // FIXME: this will break parallel test_drivers on Windows, but better than a poor PID->port mapping
	   return addr;
	}
	static bool recv(unsigned char *msg, unsigned msg_size, int sfd, HANDLE winsock_event, HANDLE notification_event)
	{
		logerror("begin socket_types::recv()\n");
	   int result;
	   SOCKET sockfd = (SOCKET)(sfd);
	   int bytes_to_get = msg_size;
	   for (;;) {
			::WSAEventSelect(sockfd, winsock_event, FD_READ);
			HANDLE wait_events[2];
			wait_events[0] = winsock_event;
			wait_events[1] = notification_event;
			// 30 second timeout
			result = ::WaitForMultipleObjects(2, wait_events, FALSE, 30000);

	if(result == WAIT_TIMEOUT) {
		logerror("WaitForMultipleObjects timed out\n");
		return false;
	}
	if(result == WAIT_FAILED || result == WAIT_ABANDONED) {
		logerror("WaitForMultipleObjects failed\n");
		return false;
	}
	int which_event = (result - WAIT_OBJECT_0);
	switch(which_event)
	{
		// notification
	case 1:
		{
			 bool result = Process::handleEvents(true);
			 if (!result) {
				logerror("Failed to handle process events\n");
				return false;
			 }
		   //logerror("handled events\n");
			}
		 break;
	case 0:
		{
				logerror("recv() looking for %d bytes\n", bytes_to_get);
			   result = ::recv(sockfd, (char *)(msg), bytes_to_get, 0);
			   if(result > 0)
			   {
				   logerror("got %d bytes\n", result);
					bytes_to_get -= result;
					msg += result;
			   }
			   else if (result == SOCKET_ERROR) {
				   int e = WSAGetLastError();
				   if(e != WSAEWOULDBLOCK)
				   {
					  logerror("unable to receive message: %d\n", e);
					  return false;
				   }
			   } else {
				   logerror("socket closed\n");
				break;
			   }
			if(bytes_to_get == 0)
			{
			   logerror("received message: %s\n", msg);
			   return true;
			}
		}
		break;
   }
	   }

	}
	static int close(SOCKET s, HANDLE winsock_event)
	{
		// set socket back to blocking
		::WSAEventSelect(s, NULL, 0);
		char truth = 1;
		::setsockopt(s, SOL_SOCKET, SO_DONTLINGER, &truth, 4);
		return ::closesocket(s);
	}
	typedef int socklen_t;
};
#endif

TEST_DLL_EXPORT ComponentTester *componentTesterFactory()
{
   return (ComponentTester *) new ProcControlComponent();
}

ProcControlMutator::ProcControlMutator()
{
}

ProcControlMutator::~ProcControlMutator()
{
}

test_results_t ProcControlMutator::setup(ParameterDict &param)
{
   comp = (ProcControlComponent *) param["ProcControlComponent"]->getPtr();
   return PASSED;
}

test_results_t ProcControlMutator::pre_init(ParameterDict &param)
{
   return PASSED;
}

ProcControlComponent::ProcControlComponent() :
   sockfd(0),
   sockname(NULL),
   notification_fd(-1),
   num_processes(0),
   num_threads(0)
{
   notification_fd = evNotify()->getFD();
#if defined(os_windows_test)
   WORD wsVer = MAKEWORD(2,2);
   WSAData ignored;
   ::WSAStartup(wsVer, &ignored);
   winsock_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
}

uint64_t ProcControlComponent::adjustFunctionEntryAddress(Process::const_ptr proc, uint64_t entrypoint)
{
   //
   // On ABIv2 of the PPC64, each function has two entry points.
   // For this ABI, the first (global) entry point is 16 bytes
   // before the second (local) entry point.  The symbol table 
   // contains the address of the global entry point.
   //
   // In order for us to establish a breakpoint for calls to either
   // entrypoint, we add an 16-byte offset to the address that 
   // the symbol table gives us and break within the local entry point.
   //
   if (proc->getArchitecture() == Dyninst::Architecture::Arch_ppc64) {
      SymReader *rdr = proc->getSymbolReader()->openSymbolReader(proc->libraries().getExecutable()->getName());
      int major, minor;

      if (rdr->getABIVersion(major, minor)) {
         if (major >= 2)
            entrypoint += 16;
      }
   }
   return entrypoint;
}

static ProcControlComponent *pccomp = NULL;
static Process::cb_ret_t eventCounterFunction(Event::const_ptr ev)
{
   pccomp->eventsRecieved[ev->getEventType()].push_back(ev);
   return Process::cbDefault;
}

bool ProcControlComponent::registerEventCounter(EventType et)
{
   pccomp = this;
   return Process::registerEventCallback(et, eventCounterFunction);
}

bool ProcControlComponent::checkThread(const Thread &thread)
{
   return true;
}

Process::cb_ret_t setSocketOnLibLoad(Event::const_ptr ev)
{
   EventLibrary::const_ptr lib_ev = ev->getEventLibrary();
   bool have_libc = false;
   for (set<Library::ptr>::const_iterator i = lib_ev->libsAdded().begin(); i != lib_ev->libsAdded().end(); i++) {
      Library::ptr lib = *i;
      if (lib->getName().find("libc-") != string::npos || lib->getName().find("libc.") != string::npos) {
         have_libc = true;
         break;
      }
   }
   if (have_libc) {
      ProcControlComponent::initializeConnectionInfo(ev->getProcess());
   }
   return Process::cbDefault;
}

bool ProcControlComponent::waitForSignalFD(int signal_fd)
{
#if !defined(os_windows_test)
   fd_set rd;
   FD_ZERO(&rd);
   FD_SET(signal_fd, &rd);
   struct timeval timeout;
   timeout.tv_sec = RECV_TIMEOUT;
   timeout.tv_usec = 0;

   int result = select(signal_fd+1, &rd, NULL, NULL, &timeout);
   if (result == -1) {
      perror("Error during signal_fd select");
      return false;
   }
   if (result == 0) {
      logerror("Timeout while waiting for signal_fd\n");
      return false;
   }

   char c;
   read(signal_fd, &c, sizeof(char));
#endif
   return true;
}

void ProcControlComponent::setupStatTest(std::string exec_name)
{
   //Bad hack, but would have required significant
   //changes to testsuite otherwise
   if (strstr(exec_name.c_str(), "pc_stat")) {
      LibraryTracking::setDefaultTrackLibraries(false);
      ThreadTracking::setDefaultTrackThreads(false);
      LWPTracking::setDefaultTrackLWPs(false);
      check_threads_on_startup = false;
   }
   else {
      LibraryTracking::setDefaultTrackLibraries(true);
      ThreadTracking::setDefaultTrackThreads(true);
      LWPTracking::setDefaultTrackLWPs(true);
   }
}

ProcessSet::ptr ProcControlComponent::startMutateeSet(RunGroup *group, ParameterDict &params)
{
   ProcessSet::ptr procset;
   bool do_create = (group->createmode == CREATE);
   bool waitfor_attach = (group->createmode == USEATTACH);

   if (do_create) {
      vector<ProcessSet::CreateInfo> cinfo;
      for (unsigned i=0; i<num_processes; i++) {
         ProcessSet::CreateInfo ci;
         getMutateeParams(group, params, ci.executable, ci.argv);
         ci.error_ret = err_none;
         cinfo.push_back(ci);
         setupStatTest(ci.executable);
      }
      procset = ProcessSet::createProcessSet(cinfo);
      if (!procset) {
         logerror("Failed to execute new mutatees\n");
         return ProcessSet::ptr();
      }
   }
   else {
      vector<ProcessSet::AttachInfo> ainfo;
      for (unsigned i=0; i<num_processes; i++) {
         ProcessSet::AttachInfo ai;
         vector<string> argv;
         getMutateeParams(group, params, ai.executable, argv);
         setupStatTest(ai.executable);
         ai.pid = getMutateePid(group);

         if (ai.pid == NULL_PID) {
            string mutateeString = launchMutatee(ai.executable, argv, group, params);
            if (mutateeString == string("")) {
               logerror("Error creating attach process\n");
               return ProcessSet::ptr();
            }
            registerMutatee(mutateeString);
            ai.pid = getMutateePid(group);
         }
         assert(ai.pid != NULL_PID);
         ainfo.push_back(ai);

         if (waitfor_attach) {
            int signal_fd = params.find("signal_fd_in") != params.end() ? params["signal_fd_in"]->getInt() : -1;
            if (signal_fd > 0) {
               bool result = waitForSignalFD(signal_fd);
               if (!result) {
                  logerror("Timeout waiting for signalFD\n");
                  return ProcessSet::ptr();
               }
            }
         }
      }

      procset = ProcessSet::attachProcessSet(ainfo);
      if (!procset) {
         logerror("Failed to attach to new mutatees\n");
         return ProcessSet::ptr();
      }
   }

   assert(procset);
   for (ProcessSet::iterator i = procset->begin(); i != procset->end(); i++) {
      Process::ptr proc = *i;
      Dyninst::PID pid = proc->getPid();
      process_pids[pid] = proc;
      procs.push_back(proc);
   }

   return procset;
}

#define MAX_ARGS 128
Process::ptr ProcControlComponent::startMutatee(RunGroup *group, ParameterDict &params)
{
   vector<string> vargs;
   string exec_name;
   getMutateeParams(group, params, exec_name, vargs);
   setupStatTest(exec_name);

   Process::ptr proc = Process::ptr();
   if (group->createmode == CREATE) {
      proc = Process::createProcess(exec_name, vargs);
      if (!proc) {
         logerror("Failed to execute new mutatee\n");
         return Process::ptr();
      }
   }
   else if (group->createmode == USEATTACH) {
      Dyninst::PID pid = getMutateePid(group);
      if (pid == NULL_PID) {
         string mutateeString = launchMutatee(exec_name, vargs, group, params);
         if (mutateeString == string("")) {
            logerror("Error creating attach process\n");
            return Process::ptr();
         }
         registerMutatee(mutateeString);
         pid = getMutateePid(group);
      }
      assert(pid != NULL_PID);

      int signal_fd = params.find("signal_fd_in") != params.end() ? params["signal_fd_in"]->getInt() : -1;
      if (signal_fd > 0) {
         bool result = waitForSignalFD(signal_fd);
         if (!result) {
            logerror("Timeout waiting for signalFD\n");
            return Process::ptr();
         }
      }
      proc = Process::attachProcess(pid, group->mutatee);
	  if (!proc) {
         logerror("Failed to attach to new mutatee\n");
         return Process::ptr();
      }
   }
   else {
      return Process::ptr();
   }

   assert(proc);
   Dyninst::PID pid = proc->getPid();
   process_pids[pid] = proc;
   procs.push_back(proc);
   return proc;
}

#if !defined(os_windows_test)
void setupSignalFD(ParameterDict &param)
{
   int fds[2];
   int result = pipe(fds);
   if (result == -1) {
      perror("Pipe error");
      exit(-1);
   }
   param["signal_fd_in"] = new ParamInt(fds[0]);
   param["signal_fd_out"] = new ParamInt(fds[1]);
}

void resetSignalFD(ParameterDict &param)
{
   if (param.find("signal_fd_in") != param.end()) {
      close(param["signal_fd_in"]->getInt());
   }
   if (param.find("signal_fd_out") != param.end()) {
      close(param["signal_fd_out"]->getInt());
   }
}
#endif

static char socket_buffer[4096];
static RunGroup *cur_group = NULL;
static SymbolReaderFactory *factory = NULL;

bool ProcControlComponent::initializeConnectionInfo(Process::const_ptr proc)
{
   static map<string, Offset> cached_ms_addrs;

   SymReader *reader = NULL;
   Dyninst::Offset sym_offset = 0;
   Dyninst::Offset exec_addr = 0;
   std::string exec_name;

   Library::const_ptr lib = proc->libraries().getExecutable();
   if (lib == Library::const_ptr()) {
      exec_name = cur_group->mutatee;
      exec_addr = 0;
   }
   else {
      exec_name = lib->getName();
      exec_addr = lib->getLoadAddress();
   }

   map<string, Offset>::iterator i = cached_ms_addrs.find(exec_name);
   if (i != cached_ms_addrs.end()) {
      sym_offset = i->second;
   }
   else {
      reader = factory->openSymbolReader(exec_name);
      if (!reader) {
         logerror("Could not open executable %s\n", exec_name.c_str());
         return false;
      }
      Symbol_t sym = reader->getSymbolByName(string("MutatorSocket"));
      if (!reader->isValidSymbol(sym))
      {
         logerror("Could not find MutatorSocket symbol in executable\n");
         return false;
      }
      sym_offset = reader->getSymbolOffset(sym);
      cached_ms_addrs[exec_name] = sym_offset;
   }

   Dyninst::Address addr = exec_addr + sym_offset;
   bool result = proc->writeMemory(addr, socket_buffer, strlen(socket_buffer)+1);
   if (!result) {
      logerror("Could not write connection information\n");
      return false;
   }
   return true;
}

bool ProcControlComponent::startMutatees(RunGroup *group, ParameterDict &param)
{
   bool error = false;
   bool result;

   if (group->procmode == MultiProcess)
      num_processes = getNumProcs(param);
   else
      num_processes = 1;
#if !defined(os_windows_test)
   setupSignalFD(param);
#endif
   check_threads_on_startup = true;
   Process::ptr a_proc;
   if (num_processes > 1) {
      pset = startMutateeSet(group, param);
      if (pset) {
         assert(!pset->empty());
         a_proc = *(pset->begin());
      }
   }
   else {
      a_proc = startMutatee(group, param);
      pset = ProcessSet::newProcessSet(a_proc);
   }
   assert(a_proc);
   factory = a_proc->getSymbolReader();
   assert(factory);

   /**
    * Set the socket name in each process
    **/
   assert(num_processes);
   memset(socket_buffer, 0, 4096);
   if (param.find("socket_type") != param.end() && param.find("socket_name") != param.end()) {
      snprintf(socket_buffer, 4095, "%s %s", param["socket_type"]->getString(),
               param["socket_name"]->getString());
   }
   cur_group = group;

   for (vector<Process::ptr>::iterator j = procs.begin(); j != procs.end(); j++) {
      bool result = initializeConnectionInfo(*j);
      if (!result) {
         error = true;
      }
   }

   EventType thread_create(EventType::None, EventType::ThreadCreate);
   registerEventCounter(thread_create);

   num_threads = group->threadmode == MultiThreaded ? getNumThreads(param) : 0;
   int num_procs = pset->size();
   result = pset->continueProcs();
   if (!result) {
      logerror("Error doing initial continueProcs");
      error = true;
   }

   result = acceptConnections(num_procs, NULL);
   if (!result) {
      logerror("Failed to accept connections from new mutatees\n");
      error = true;
   }

   if (group->createmode == CREATE)
   {
      Process::ptr a_proc = *procs.begin();

      bool support_user_threads = a_proc->supportsUserThreadEvents();
      bool support_lwps = a_proc->supportsLWPEvents();

      assert(support_user_threads || support_lwps);

      if (support_lwps && check_threads_on_startup)
      {
         while (eventsRecieved[EventType(EventType::None, EventType::LWPCreate)].size() < num_procs*num_threads) {
            bool result = Process::handleEvents(true);
            if (!result) {
               logerror("Failed to handle events during thread create\n");
               error = true;
			   return false;
            }
         }
      }

      if (support_user_threads && check_threads_on_startup)
      {
         while (eventsRecieved[EventType(EventType::None, EventType::UserThreadCreate)].size() < num_procs*num_threads) {
            bool result = Process::handleEvents(true);
            if (!result) {
               logerror("Failed to handle events during thread create\n");
               error = true;
			   return false;
            }
         }
      }
   }
   else if (group->createmode == USEATTACH)
   {
      for (std::vector<Process::ptr>::iterator j = procs.begin(); j != procs.end(); j++) {
         Process::ptr proc = *j;
         if (proc->threads().size() != num_threads+1) {
            logerror("Process has incorrect number of threads");
            error = true;
         }
      }
      if (eventsRecieved[thread_create].size()) {
         logerror("Recieved unexpected thread creation events on process\n");
         error = true;
      }
   }

   if (group->state != RUNNING && check_threads_on_startup) {
      bool result = pset->stopProcs();
      if (!result) {
         logerror("Failed to stop process");
         error = true;
      }
   }

   handshake shake;
   shake.code = HANDSHAKE_CODE;
   result = send_broadcast((unsigned char *) &shake, sizeof(handshake));
   if (!result) {
      logerror("Failed to send handshake message to processes\n");
      error = true;
   }

   return !error;
}



test_results_t ProcControlComponent::program_setup(ParameterDict &params)
{
	setupServerSocket(params);
	return PASSED;
}

test_results_t ProcControlComponent::program_teardown(ParameterDict &params)
{

	cleanSocket();
   return PASSED;
}

test_results_t ProcControlComponent::group_setup(RunGroup *group, ParameterDict &params)
{
   process_socks.clear();
   process_pids.clear();
   procs.clear();
   eventsRecieved.clear();
   curgroup_self_cleaning = false;

   me.setPtr(this);
   params["ProcControlComponent"] = &me;

   for (unsigned j=0; j<group->tests.size(); j++) {
      ProcControlMutator *mutator = static_cast<ProcControlMutator *>(group->tests[j]->mutator);
      if (!mutator) continue;
      test_results_t result = mutator->pre_init(params);
      if (result == FAILED)
         return FAILED;
   }

   bool result = startMutatees(group, params);
   if (!result) {
      logerror("Failed to launch mutatees\n");
      return FAILED;
   }

   return PASSED;
}

Process::cb_ret_t default_on_exit(Event::const_ptr ev)
{
  logerror("Got exit event for process %d\n", ev->getProcess()->getPid());
   return Process::cbDefault;
}

test_results_t ProcControlComponent::group_teardown(RunGroup *group, ParameterDict &params)
{
   bool error = false;
   bool hasRunningProcs;
#if !defined(os_windows_test)
   resetSignalFD(params);
#endif
   logerror("Begin ProcControl group teardown\n");
   for(std::map<Process::ptr, int>::iterator i = process_socks.begin(); i != process_socks.end(); ++i) {
#if defined(os_windows_test)
	   if( socket_types::close(i->second, winsock_event) == SOCKET_ERROR ) {
#else
	   if( socket_types::close(i->second) == SOCKET_ERROR ) {
#endif
		   logerror("Could not close connected socket\n");
           error = true;
       }
   }
   logerror("Sockets all closed\n");

   if (curgroup_self_cleaning) {
     logerror("Self cleaning group, we're done here\n");
      procs.clear();
      return PASSED;
   }

   bool allSkippedTests = true;
   for (unsigned j=0; j<group->tests.size(); j++) {
       if (group->tests[j]->results[test_execute_rs] != SKIPPED) {
           allSkippedTests = false;
       }
   }

   if (allSkippedTests) {
       // If all tests are skipped.
       // Then the mutatee may be left in attached status forever.
       // We can simply terminate the mutatees.
       for (std::vector<Process::ptr>::iterator i = procs.begin(); i != procs.end(); i++) {
           Process::ptr p = *i;
           p->terminate();
       }
       return SKIPPED;
   }
   Process::registerEventCallback(EventType(EventType::Post, EventType::Exit), default_on_exit);
   do {
      hasRunningProcs = false;
      for (std::vector<Process::ptr>::iterator i = procs.begin(); i != procs.end(); i++) {
         Process::ptr p = *i;
         while (!p->isTerminated()) {
	   logerror("Process %d not terminated, is %s, is %s, blocking for events\n",
		    p->getPid(),
		    p->allThreadsStopped() ? "stopped" : "running",
		    p->isExited() ? "exited" : "not exited");
            bool result = block_for_events();
            if (!result) {
               logerror("Process failed to handle events\n");
			   error = true;
			   continue;
            }
            if (!p->isTerminated()) {
               hasRunningProcs = true;
            }
         }
      }
   } while(hasRunningProcs);

   for (std::vector<Process::ptr>::iterator i = procs.begin(); i != procs.end(); i++) {
      Process::ptr p = *i;
      if (!p->isTerminated()) {
         logerror("Process did not terminate\n");
         error = true;
         continue;
      }
      if (p->isCrashed()) {
         logerror("Process terminated on crash\n");
         error = true;
         continue;
      }
      if (!p->isExited()) {
         logerror("Process did not report as exited\n");
         error = true;
         continue;
      }
      if (p->getExitCode() != 0) {
         logerror("Process has unexpected error code: 0x%lx\n", p->getExitCode());
         error = true;
         continue;
      }
   }
   procs.clear();

   return error ? FAILED : PASSED;
}

test_results_t ProcControlComponent::test_setup(TestInfo *test, ParameterDict &parms)
{
   return PASSED;
}

test_results_t ProcControlComponent::test_teardown(TestInfo *test, ParameterDict &parms)
{
   return PASSED;
}

std::string ProcControlComponent::getLastErrorMsg()
{
   return std::string("");
}

ProcControlComponent::~ProcControlComponent()
{
#if defined(os_windows_test)
	::WSACleanup();
	::CloseHandle(winsock_event);
#endif
}


void handleError(const char* msg)
{
	char details[1024];
#if defined(os_windows_test)
	int err = WSAGetLastError();
    ::FormatMessage(0, NULL, err, 0, details, 1024, NULL);
#else
	strncpy(details, strerror(errno), 1024);
#endif
	fprintf(stderr, "handleError: %s\n", details);
	logerror(msg, details);
}

bool ProcControlComponent::setupServerSocket(ParameterDict &param)
{
	SOCKET fd = INVALID_SOCKET; // initialize, dammit.
	fd = socket_types::socket();
   if (fd == INVALID_SOCKET) {
	   handleError("Failed to create socket: %s\n");
      return false;
   }
   socket_types::sockaddr_t addr = socket_types::make_addr();

   int timeout = RECV_TIMEOUT * 100;
   int result;
   for (;;) {
      result = ::bind(fd, (sockaddr *) &addr, sizeof(socket_types::sockaddr_t));
      if (result == 0)
         break;
      int error = errno;
#if !defined(os_windows_test)
      if (error == EADDRINUSE && timeout) {
         timeout--;
         usleep(10000);
         continue;
      }
#endif
      if (result != 0){
         handleError("Unable to bind socket: %s\n");
         closesocket(fd);
         return false;
      }
   }
   result = listen(fd, 512);
   if (result == -1) {
	   handleError("Unable to listen on socket: %s\n");
	  closesocket(fd);
      return false;
   }

   sockfd = fd;
   sockname = new char[1024];
#if defined(os_windows_test)
   snprintf(sockname, 1023, "/tmp/pct%d", addr.sin_port);
   char *socket_type = "inet_socket";
#else
   snprintf(sockname, 1023, "/tmp/pct%d", getpid());
   const char *socket_type = "un_socket";
#endif
   param["socket_type"] = new ParamString(socket_type);
   param["socket_name"] = new ParamString(strdup(sockname));
   param["socketfd"] = new ParamInt(sockfd);
   return true;
}

#if !defined(os_windows_test)
bool ProcControlComponent::acceptConnections(int num, int *attach_sock)
{
   vector<int> socks;
   assert(num == 1 || !attach_sock);  //If attach_sock, then num == 1

   while (socks.size() < num) {
      fd_set readset; FD_ZERO(&readset);
      fd_set writeset; FD_ZERO(&writeset);
      fd_set exceptset; FD_ZERO(&exceptset);

      FD_SET(sockfd, &readset);
      FD_SET(notification_fd, &readset);
      int nfds = (sockfd > notification_fd ? sockfd : notification_fd)+1;

      struct timeval timeout;
      timeout.tv_sec = RECV_TIMEOUT;
      timeout.tv_usec = 0;
      int result = select(nfds, &readset, &writeset, &exceptset, &timeout);
      if (result == 0) {
         logerror("Timeout while waiting for socket connect");
         fprintf(stderr, "[%s:%u] - Have received %lu / %d socks\n", __FILE__, __LINE__, socks.size(), num);
         return false;
      }
      if (result == -1) {
         perror("Error in select");
         return false;
      }

      if (FD_ISSET(sockfd, &readset))
      {
         struct sockaddr_un addr;
         socklen_t addr_size = sizeof(struct sockaddr_un);
         int newsock = accept(sockfd, (struct sockaddr *) &addr, &addr_size);
         if (newsock == -1) {
            char error_str[1024];
            snprintf(error_str, 1024, "Unable to accept socket: %s\n", strerror(errno));
            logerror(error_str);
            return false;
         }
         socks.push_back(newsock);
      }
      if (FD_ISSET(notification_fd, &readset)) {
         bool result = Process::handleEvents(true);
         if (!result) {
            logerror("Failed to handle process events\n");
            return false;
         }
      }
   }

   for (unsigned i=0; i<num; i++) {
      send_pid msg;
      bool result;
      result = recv_message((unsigned char *) &msg, sizeof(send_pid), socks[i]);
      if (!result) {
         logerror("Could not receive handshake pid\n");
         return false;
      }
      if (msg.code != SEND_PID_CODE)
      {
         logerror("Received bad code in handshake message\n");
         return false;
      }
      int pid = msg.pid;

      map<Dyninst::PID, Process::ptr>::iterator j = process_pids.find(pid);
      if (j == process_pids.end()) {
         if (attach_sock) {
            *attach_sock = socks[i];
            return true;
         }
         logerror("Recieved unexpected PID (%d) in handshake message\n", msg.pid);
         return false;
      }
      process_socks[j->second] = socks[i];
   }

   return true;
}
#else
// Windows
bool ProcControlComponent::acceptConnections(int num, int *attach_sock)
{
   std::vector<int> socks;
   assert(sockfd);
   assert(num == 1 || !attach_sock);  //If attach_sock, then num == 1

   while (socks.size() < num) {
	HANDLE notification_event = (HANDLE) notification_fd;

	::WSAEventSelect(sockfd, winsock_event, FD_ACCEPT);
	HANDLE wait_events[2];
	wait_events[0] = winsock_event;
	wait_events[1] = notification_event;
	// 30 second timeout
	int result = ::WaitForMultipleObjects(2, wait_events, FALSE, 30000);

	if(result == WAIT_TIMEOUT) {
		handleError("WaitForMultipleObjects timed out\n");
		return false;
	}
	if(result == WAIT_FAILED || result == WAIT_ABANDONED) {
		handleError("WaitForMultipleObjects failed\n");
		return false;
	}
	int which_event = (result - WAIT_OBJECT_0);
	switch(which_event)
	{
		// notification
	case 1:
		{
			 bool result = Process::handleEvents(true);
			 if (!result) {
				handleError("Failed to handle process events\n");
				return false;
			 }
			}
		 break;
	case 0:
		{
		  socket_types::sockaddr_t addr;
         socket_types::socklen_t addr_size = sizeof(socket_types::sockaddr_t);
         int newsock = accept(sockfd, (struct sockaddr *) &addr, &addr_size);
		 if (newsock == -1) {
			 if (::WSAGetLastError() == WSAEWOULDBLOCK) {
				 continue;
			 }
            char error_str[1024];
            snprintf(error_str, 1024, "Unable to accept socket: %d/%d, %s, %d\n", errno, WSAGetLastError(), strerror(errno), sockfd);
            logerror(error_str);
            return false;
         }
         socks.push_back(newsock);
		}
		break;
	}
   }
   for (int i=0; i<num; i++) {
      send_pid msg;
      bool result = recv_message((unsigned char *) &msg, sizeof(send_pid), socks[i]);
      if (!result) {
         logerror("Could not receive handshake pid\n");
         return false;
      }
      if (msg.code != SEND_PID_CODE)
      {
         logerror("Received bad code in handshake message\n");
         return false;
      }
      std::map<Dyninst::PID, Process::ptr>::iterator j = process_pids.find(msg.pid);
      if (j == process_pids.end()) {
         if (attach_sock) {
            *attach_sock = socks[i];
            return true;
         }
         logerror("Recieved unexpected PID in handshake message\n");
         return false;
      }
      process_socks[j->second] = socks[i];
   }

	return true;
}

#endif

bool ProcControlComponent::cleanSocket()
{
   if (!sockname)
      return false;

   int result;
#if !defined(os_windows_test)
   result = unlink(sockname);
   if (result == -1) {
      logerror("Could not clean socket\n");
      return false;
   }
#endif
   delete[] sockname;
   sockname = NULL;
#if defined(os_windows_test)
   result = socket_types::close(sockfd, winsock_event);
#else
   result = socket_types::close(sockfd);
#endif
   if (result == -1) {
      logerror("Could not close socket\n");
      return false;
   }
   return true;
}

bool ProcControlComponent::recv_message(unsigned char *msg, unsigned msg_size, Process::ptr p)
{
  return recv_message(msg, msg_size, process_socks[p]);
}

bool ProcControlComponent::send_message(unsigned char *msg, unsigned msg_size, Process::ptr p)
{
  return send_message(msg, msg_size, process_socks[p]);
}

bool ProcControlComponent::recv_message(unsigned char *msg, unsigned msg_size, int sfd)
{
#if defined(os_windows_test)
	return socket_types::recv(msg, msg_size, sfd, winsock_event, (HANDLE)(notification_fd));
#else
	return socket_types::recv(msg, msg_size, sfd, notification_fd);
#endif
}

bool ProcControlComponent::recv_broadcast(unsigned char *msg, unsigned msg_size)
{
   assert(!process_pids.empty());
   unsigned char *cur_pos = msg;
   for (std::map<Dyninst::PID, Process::ptr>::iterator i = process_pids.begin(); i != process_pids.end(); i++) {
      bool result = recv_message(cur_pos, msg_size, i->second);
      if (!result)
         return false;
      cur_pos += msg_size;
   }
   return true;
}

bool ProcControlComponent::send_message(unsigned char *msg, unsigned msg_size, int sfd)
{
   int result = send(sfd, (char*)(msg), msg_size, MSG_NOSIGNAL);
   if (result == -1) {
      char error_str[1024];
      snprintf(error_str, 1024, "Mutator unable to send message: %s\n", strerror(errno));
      logerror(error_str);
      return false;
   }
   return true;
}

bool ProcControlComponent::send_broadcast(unsigned char *msg, unsigned msg_size)
{
   assert(!process_pids.empty());
   unsigned char *cur_pos = msg;
   for (std::map<Dyninst::PID, Process::ptr>::iterator i = process_pids.begin(); i != process_pids.end(); i++) {
      bool result = send_message(msg, msg_size, i->second);
      if (!result)
         return false;
   }
   return true;
}

bool ProcControlComponent::block_for_events()
{
#if !defined(os_windows_test)
   int nfds = notification_fd+1;
   fd_set readset; FD_ZERO(&readset);
   fd_set writeset; FD_ZERO(&writeset);
   fd_set exceptset; FD_ZERO(&exceptset);
   FD_SET(notification_fd, &readset);

   struct timeval timeout;
   timeout.tv_sec = 15;
   timeout.tv_usec = 0;
   int result;
   do {
      result = select(nfds, &readset, &writeset, &exceptset, &timeout);
   } while (result == -1 && errno == EINTR);

   if (result == 0) {
      logerror("Timeout while waiting for event\n");
      return false;
   }
   if (result == -1) {
      char error_str[1024];
      snprintf(error_str, 1024, "Error calling select: %s\n", strerror(errno));
      logerror(error_str);
      return false;
   }

   assert(result == 1 && FD_ISSET(notification_fd, &readset));
   bool bresult = Process::handleEvents(true);
   if (!bresult) {
      logerror("Error waiting for events\n");
      return false;
   }
   return true;
#else
	int result = ::WaitForSingleObject((HANDLE)(notification_fd), 15000);
	if(result == WAIT_TIMEOUT) {
        logerror("Timeout while waiting for event\n");
		return false;
	}
	if(result != WAIT_OBJECT_0) {
		logerror("Error waiting for notify\n");
		return false;
	}
	bool proc_result = Process::handleEvents(true);
	if(!proc_result) {
		logerror("Error waiting for events\n");
		return false;
	}
	logerror("block_for_events returning OK\n");
	return true;
#endif
}

bool ProcControlComponent::poll_for_events()
{
   bool bresult = Process::handleEvents(false);
   return bresult;
}

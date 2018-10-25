//
// Created by bill on 2/15/18.
//

#ifndef DYNINST_USAGEMONITOR_H
#define DYNINST_USAGEMONITOR_H

#include "test_lib_dll.h"

#if !defined(os_windows_test)

#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>

class UsageMonitor
{
    enum useProcState {
        PS_SKIP,
        PS_UNKNOWN,
        PS_USE
    };

    enum usageMonitorState {
        UM_CLEAR,
        UM_HASDATA,
        UM_COMPLETE
    };

public:
    TESTLIB_DLL_EXPORT UsageMonitor();
    TESTLIB_DLL_EXPORT void start();
    TESTLIB_DLL_EXPORT void end();
    TESTLIB_DLL_EXPORT void clear();
    TESTLIB_DLL_EXPORT void set(timeval &);
    TESTLIB_DLL_EXPORT void set(unsigned long);
    TESTLIB_DLL_EXPORT void complete();
    TESTLIB_DLL_EXPORT bool has_data() const;

    TESTLIB_DLL_EXPORT const timeval &cpuUsage() const;
    TESTLIB_DLL_EXPORT unsigned long memUsage() const;

    TESTLIB_DLL_EXPORT UsageMonitor &operator=(const UsageMonitor &);
    TESTLIB_DLL_EXPORT UsageMonitor &operator+=(const UsageMonitor &);
    TESTLIB_DLL_EXPORT const UsageMonitor operator+(const UsageMonitor &) const;

private:
    void mark(struct rusage *ru);

    static useProcState use_proc;
    struct rusage start_usage;

    timeval total_cpu;
    unsigned long total_mem;
    usageMonitorState state;
};

#else

// Empty implementation for Windows

#include <winsock2.h>  // For struct timeval

class UsageMonitor
{
public:
  TESTLIB_DLL_EXPORT UsageMonitor() {};
  TESTLIB_DLL_EXPORT void start() {};
  TESTLIB_DLL_EXPORT void end() {};
  TESTLIB_DLL_EXPORT void clear() {};
  TESTLIB_DLL_EXPORT void set(timeval &) {};
  TESTLIB_DLL_EXPORT void set(unsigned long) {};
  TESTLIB_DLL_EXPORT void complete() {};
  TESTLIB_DLL_EXPORT bool has_data() const { return false; };

  TESTLIB_DLL_EXPORT timeval cpuUsage() const { return timeval(); };
  TESTLIB_DLL_EXPORT unsigned long memUsage() const { return 0; };

  TESTLIB_DLL_EXPORT UsageMonitor &operator=(const UsageMonitor &) { return *this; };
  TESTLIB_DLL_EXPORT UsageMonitor &operator+=(const UsageMonitor &) { return *this; };
  TESTLIB_DLL_EXPORT const UsageMonitor operator+(const UsageMonitor &) const { return UsageMonitor(*this); };
};

#endif



#endif //DYNINST_USAGEMONITOR_H

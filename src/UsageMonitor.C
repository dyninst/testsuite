//
// Created by bill on 2/15/18.
//

#include "UsageMonitor.h"

#if !defined(os_windows_test)

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>

#ifndef timersub
#define timersub(b, a, r) \
do { \
    (r)->tv_sec = (b)->tv_sec - (a)->tv_sec;\
    (r)->tv_usec = (b)->tv_usec - (a)->tv_usec;\
    if((r)->tv_usec < 0) {\
        (r)->tv_sec--;\
        (r)->tv_usec += 1000000;\
    } \
} while(0)
#endif

#ifndef timeradd
#define timeradd(b, a, r) \
do { \
    (r)->tv_sec = (b)->tv_sec + (a)->tv_sec;\
    (r)->tv_usec = (b)->tv_usec + (a)->tv_usec;\
    if((r)->tv_usec > 1000000) {\
        (r)->tv_sec += (r)->tv_usec / 1000000;\
        (r)->tv_usec = (r)->tv_usec % 1000000;\
    } \
} while(0)
#endif

#endif


UsageMonitor::useProcState UsageMonitor::use_proc = PS_UNKNOWN;

UsageMonitor::UsageMonitor()
{
    if (use_proc == PS_UNKNOWN) {
        struct stat s;
        if (stat("/proc/self/status", &s) == 0)
            use_proc = PS_USE;
        else
            use_proc = PS_SKIP;
    }
    clear();
}

void UsageMonitor::clear()
{
    total_mem = 0;
    total_cpu.tv_sec = 0;
    total_cpu.tv_usec = 0;
    state = UM_CLEAR;
}

void UsageMonitor::start()
{
    if (state == UM_COMPLETE) {
        fprintf(stderr, "*** Refusing to start completed UsageMonitor\n");
        return;
    }
    mark(&start_usage);
}

void UsageMonitor::end()
{
    if (state == UM_COMPLETE) return;

    struct rusage end_usage;
    mark(&end_usage);

    timersub(&end_usage.ru_utime, &start_usage.ru_utime, &end_usage.ru_utime);
    timeradd(&end_usage.ru_utime, &total_cpu, &total_cpu);
    timersub(&end_usage.ru_stime, &start_usage.ru_stime, &end_usage.ru_stime);
    timeradd(&end_usage.ru_stime, &total_cpu, &total_cpu);
    total_mem += (end_usage.ru_maxrss - start_usage.ru_maxrss);
    state = UM_HASDATA;
}

void UsageMonitor::set(timeval &cpu)
{
    total_cpu = cpu;
    state = UM_HASDATA;
}

void UsageMonitor::set(unsigned long mem)
{
    total_mem = mem;
    state = UM_HASDATA;
}

void UsageMonitor::complete()
{
    state = UM_COMPLETE;
}

bool UsageMonitor::has_data() const
{
    return state != UM_CLEAR;
}

const timeval &UsageMonitor::cpuUsage() const
{
    return total_cpu;
}

unsigned long UsageMonitor::memUsage() const
{
    return total_mem;
}

UsageMonitor &UsageMonitor::operator=(const UsageMonitor &rhs)
{
    if (this != &rhs) {
        total_cpu = rhs.total_cpu;
        total_mem = rhs.total_mem;
        state = rhs.state;
    }
    return *this;
}

UsageMonitor &UsageMonitor::operator+=(const UsageMonitor &rhs)
{
    if (state != UM_CLEAR && rhs.state != UM_CLEAR) {
        total_mem += rhs.total_mem;
        if (state == UM_COMPLETE || rhs.state == UM_COMPLETE)
            state = UM_COMPLETE;
        else
            state = UM_HASDATA;
    }

    return *this;
}

const UsageMonitor UsageMonitor::operator+(const UsageMonitor &rhs) const
{
    return UsageMonitor(*this) += rhs;
}

void UsageMonitor::mark(struct rusage *ru)
{
    getrusage(RUSAGE_SELF, ru);
    if (!ru->ru_maxrss && use_proc != PS_SKIP) {
        unsigned long vmRSS  = 0;
        unsigned long vmSize = 0;

        FILE *fp = fopen("/proc/self/status", "r");
        if (!fp) return;

        char buf[1024] = {0};
        char *ptr = buf, *end = buf + sizeof(buf) - 1;
        while (!feof(fp) && !ferror(fp)) {
            int i = fread(ptr, sizeof(char), end - ptr, fp);
            ptr[i+1] = '\0';

            ptr = strstr(buf, "VmRSS:");
            if (ptr) sscanf(ptr, "VmRSS: %lu", &vmRSS);
            ptr = strstr(buf, "VmSize:");
            if (ptr) sscanf(ptr, "VmSize: %lu", &vmSize);

            if (!feof(fp) && !ferror(fp)) {
                ptr = strrchr(buf, '\n');
                if (!ptr++) break;

                for (i = 0; ptr + i < end; ++i) buf[i] = ptr[i];
                ptr = buf + i;
            }
        }
        fclose(fp);

        if (vmRSS)  ru->ru_maxrss = vmRSS;
        if (vmSize) ru->ru_ixrss  = vmSize;

        if (!vmRSS && !vmSize)
            use_proc = PS_SKIP;
    }
}


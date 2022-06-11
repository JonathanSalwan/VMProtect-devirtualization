/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application is used with several tests.  It executes the global function
 * InstrumentedWithPin() in parallel with several threads.  Pin tools can add
 * instrumentation to that function to test parallel calls to an analysis routine.
 *
 * This application uses Posix API's, but there is a similar implementation
 * that works on Windows platforms.
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include "atomic.hpp"

#if defined(FUND_HOST_MAC)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

extern "C" void InstrumentedWithPin(volatile UINT32*);
extern "C" void TellPinThreadCount(ADDRINT);
extern "C" void TellPinThreadStart();

static unsigned const DEFAULT_CPU_COUNT = 2;

typedef void (*FUNPTR1)(volatile UINT32*);
typedef void (*FUNPTR2)(ADDRINT);
typedef void (*FUNPTR3)();

static volatile bool Ready = false;

static unsigned GetThreadCount(int, char**);
static unsigned GetCpuCount();
static void* Worker(void*);

int main(int argc, char** argv)
{
    unsigned numThreads = GetThreadCount(argc, argv);
    if (!numThreads) return 1;

    std::cout << "Testing with threads: " << std::dec << numThreads << std::endl;

    // Tell the Pin tool the number of worker threads.
    //
    volatile FUNPTR2 tellPinCount = TellPinThreadCount;
    tellPinCount(numThreads);

    pthread_t* thds = new pthread_t[numThreads];
    for (unsigned i = 0; i < numThreads; i++)
    {
        if (pthread_create(&thds[i], 0, Worker, 0) != 0)
        {
            std::cerr << "Unable to create worker thread #" << std::dec << i << std::endl;
            return 1;
        }
    }

    // Tell all the worker threads to go.
    //
    ATOMIC::OPS::Store(&Ready, true);

    for (unsigned i = 0; i < numThreads; i++)
        pthread_join(thds[i], 0);
    delete[] thds;
    return 0;
}

static unsigned GetThreadCount(int argc, char** argv)
{
    // If there's no explicit thread parameter, use the number of CPU's
    // but not less than 2 and not more than 8.
    //
    if (argc <= 1)
    {
        unsigned ncpus = GetCpuCount();
        if (ncpus < 2) ncpus = 2;
        if (ncpus > 8) ncpus = 8;
        return ncpus;
    }

    if (std::strcmp(argv[1], "-t") != 0)
    {
        std::cerr << "Unknown option: " << argv[1] << std::endl;
        return 0;
    }

    if (argc < 3)
    {
        std::cerr << "Must specify a parameter to '-t'" << std::endl;
        return 0;
    }

    char* end;
    unsigned long val = std::strtoul(argv[2], &end, 10);
    if (*(argv[2]) == '\0' || val > UINT_MAX || val == 0 || *end != '\0')
    {
        std::cerr << "Invalid parameter to -t: " << argv[2] << std::endl;
        return 0;
    }
    return static_cast< unsigned >(val);
}

static unsigned GetCpuCount()
{
#if defined(FUND_HOST_LINUX) || defined(FUND_HOST_BSD)

    long count = sysconf(_SC_NPROCESSORS_ONLN);
    if (count != -1) return count;

#elif defined(FUND_HOST_MAC)

    int mib[2];
    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    unsigned ncpu;
    size_t len = sizeof(ncpu);
    if (sysctl(mib, 2, &ncpu, &len, 0, 0) == 0) return ncpu;

#endif

    std::cout << "Unable to get system CPU count, using default" << std::endl;
    return DEFAULT_CPU_COUNT;
}

static void* Worker(void*)
{
    volatile FUNPTR3 threadstarted = TellPinThreadStart;
    threadstarted();
    // Wait for all the worker threads to be created.  We use an active
    // spin loop here to help ensure that all thread execute the loop below
    // in parallel.
    //
    while (!ATOMIC::OPS::Load(&Ready))
        ;

    // Call InstrumentedWithPin() through a volatile pointer to prevent the compiler
    // from inlining the body.
    //
    volatile FUNPTR1 doFun = InstrumentedWithPin;
    volatile UINT32 done   = 0;
    while (!done)
        doFun(&done);

    return 0;
}

extern "C" void InstrumentedWithPin(volatile UINT32* done)
{
    // Pin tool places instrumentation here.
    // It stores non-zero to 'done' when the thread should exit.
}

extern "C" void TellPinThreadCount(ADDRINT threadCount)
{
    // Pin tool can place instrumentation here to learn the worker thread count.
}

extern "C" void TellPinThreadStart()
{
    // Pin tool can place instrumentation here to know the worker thread started.
}

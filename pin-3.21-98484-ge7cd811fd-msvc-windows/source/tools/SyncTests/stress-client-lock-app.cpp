/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <cstdlib>
#include "atomic.hpp"

using std::cerr;
using std::cout;
using std::endl;
using std::flush;

volatile unsigned int threadCount  = 0;
volatile bool go                   = false;
volatile unsigned int stillRunning = 0;
volatile bool done                 = false;

pid_t GetTid() { return syscall(__NR_gettid); }

extern "C"
{
    void SecondaryThreadInit(unsigned int threadNum, pid_t tid)
    {
        ATOMIC::OPS::Store(&stillRunning, (unsigned int)tid);
        cout << "APP: Thread #" << threadNum << " (" << tid << ") started." << endl << flush;
    }

    static void SecondaryThreadWork()
    {
        ATOMIC::OPS::Store(&stillRunning, (unsigned int)GetTid());
        sched_yield();
    }

    void SecondaryThreadFini(unsigned int threadNum, pid_t tid)
    {
        ATOMIC::OPS::Store(&stillRunning, (unsigned int)tid);
        cout << "APP: Thread #" << threadNum << " (" << tid << ") finished." << endl << flush;
    }

} // extern "C"

static void* SecondaryThreadMain(void* v)
{
    unsigned int threadNum = ATOMIC::OPS::Increment< unsigned int >(&threadCount, 1);
    pid_t tid              = GetTid();

    // Per-thread init
    SecondaryThreadInit(threadNum, tid);
    while (!go)
        sched_yield();

    // Stress test
    for (unsigned int i = 0; i < 1000; ++i)
    {
        SecondaryThreadWork();
    }

    // Per-thread fini
    SecondaryThreadFini(threadNum, tid);
    return NULL;
}

static void* MonitorThreadMain(void* v)
{
    // Make sure that the test is not deadlocked:
    // When a thread completes a step, it assigns a non-zero value to "stillRunning" (tid or pthread id).
    // If the monitor thread wakes up and finds that "stillRunning" is zero, it assumes a deadlock and
    // terminates the application. After a successful check ("stillRunning" is true), the monitor thread
    // resets "stillRunning" to false. A 10-second sleep period should suffice even on the most overloaded
    // system.
    //
    // Special case: The monitor thread sleeps for one 10-second period before it begins to monitor the
    // application to allow the main thread to begin its work creating threads.
    sleep(10);
    unsigned int i = 0;
    while (!done)
    {
        volatile unsigned int isRunning = ATOMIC::OPS::Load(&stillRunning);
        cout << "APP: monitor iteration " << ++i << ", stillRunning: " << stillRunning << endl << flush;
        if (0 == isRunning)
        {
            cerr << "APP ERROR: Timeout reached" << endl;
            exit(5);
        }
        ATOMIC::OPS::Store(&stillRunning, (unsigned int)0);
        sleep(10);
    }
    return NULL;
}

extern "C"
{
    void ReleaseThreads(volatile bool* doRelease)
    {
        if (false == go)
        {
            cerr << "APP ERROR: The tool should have instrumented ReleaseThreads and released the threads" << endl;
            exit(3);
        }
    }
} // extern "C"

static void CreateThreads()
{
    static const unsigned int numOfThreads = 50;
    pthread_t tids[numOfThreads];
    for (unsigned int i = 0; i < numOfThreads; ++i)
    {
        if (0 != pthread_create(&tids[i], NULL, SecondaryThreadMain, (void*)i))
        {
            cerr << "APP ERROR: Failed to create secondary thread #" << i << endl;
            exit(2);
        }
        ATOMIC::OPS::Store(&stillRunning, (unsigned int)tids[i]);
    }
    cout << "APP: All threads created successfully, waiting for them to be ready." << endl;
    while (threadCount < numOfThreads)
        sched_yield();
    ReleaseThreads(&go);
    cout << "APP: All threads are ready, waiting for them to exit." << endl;
    for (unsigned int i = 0; i < numOfThreads; ++i)
    {
        if (0 != pthread_join(tids[i], NULL))
        {
            cerr << "APP ERROR: Secondary thread #" << i << " failed to join" << endl;
            exit(4);
        }
        ATOMIC::OPS::Store(&stillRunning, (unsigned int)tids[i]);
    }
    done = true;
}

int main()
{
    pthread_t monitor;
    if (0 != pthread_create(&monitor, NULL, MonitorThreadMain, NULL))
    {
        cerr << "APP ERROR: Failed to create the monitor thread" << endl;
        exit(1);
    }
    CreateThreads();
    cout << "APP: Test completed successfully." << endl;
    return 0;
}

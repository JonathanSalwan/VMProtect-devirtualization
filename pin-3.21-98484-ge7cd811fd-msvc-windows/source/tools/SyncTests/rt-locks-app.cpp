/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that PIN_LOCK's are safe to use in an application that
 * uses real-time scheduling on Linux.  Previously, PIN_LOCK was a spin lock,
 * which didn't work with real-time scheduling.  If a low priority thread held
 * a lock, a high-priority thread would just spin forever trying to acquire
 * it.
 *
 * This test tries to reproduce the failing scenario.  A "worker" thread runs
 * in a loop where Pin has instrumented the code, and the instrumentation
 * acquires and releases a PIN_LOCK.  Meanwhile, a "scheduler" thread tries to
 * lower the priority of the worker and then executes some code where Pin has also
 * added instrumentation to acquire the same PIN_LOCK.  The hope is that the
 * worker's priority will be lowered while it holds the PIN_LOCK.  Since both
 * threads are constrained to run on the same CPU, the low priority worker
 * should never give up the CPU to the high priority scheduler unless the
 * scheduler actually blocks while attempting to get the lock.
 */

#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>

// The number of times that the scheduler thread tries to lower the priority
// of the worker thread.  The test exits with success if we can complete this
// many iterations without deadlock.
//
const unsigned long NUM_SCHEDULES = 100;

#ifndef SYS_gettid
#define SYS_gettid __NR_gettid
#endif

enum PRIORITY
{
    PRIORITY_LOW,
    PRIORITY_HIGH
};

typedef void (*VOIDFUNPTR)();

// Synchronization used to wait for the worker thread to initialize itself
// before the scheduler thread starts the test.
//
pthread_mutex_t Lock;
pthread_cond_t WorkerInitialized;
bool WorkerIsInitialized = false;
pid_t WorkerTid; // O/S thread ID of the worker.

// Ensures that error messages are not printed simultaneously by two threads.
//
pthread_mutex_t ErrorLock;

// Priority levels used for "high" and "low" priority.
//
int PriorityLow;
int PriorityHigh;

bool IsError       = false; // Set true if a system call fails in worker / scheduler thread.
volatile bool Done = false; // Asynchronously set by scheduler to terminate worker loop.

static void* Worker(void*);
static void* Scheduler(void*);
extern "C" void DoWorkInstrumentedWithPin();
extern "C" void DoGetLockWithPin();
extern "C" void TellPinNotSupported();
static bool SetPriority(pid_t, PRIORITY);
static bool SetAffinity(pid_t);
static bool CheckSupported();
static pid_t GetTid();

int main()
{
    pthread_mutex_init(&Lock, 0);
    pthread_mutex_init(&ErrorLock, 0);
    pthread_cond_init(&WorkerInitialized, 0);

    // Make sure that the system supports real-time scheduling.  The test
    // passes if it is not supported, which avoids failures during automated
    // testing.
    //
    if (!CheckSupported())
    {
        std::cout << "No RT support\n";
        TellPinNotSupported();
        return 1;
    }

    // Figure out the priority levels to use for "high" and "low".
    //
    PriorityLow = sched_get_priority_min(SCHED_RR);
    if (PriorityLow < 1) PriorityLow = 1;
    PriorityHigh = PriorityLow + 1;
    if (PriorityHigh > sched_get_priority_max(SCHED_RR))
    {
        std::cerr << "Not enough priority levels" << std::endl;
        return 1;
    }

    // The main thread runs at high priority to ensure that it can wait for
    // the other threads to complete.
    //
    if (!SetPriority(GetTid(), PRIORITY_HIGH)) return 1;

    // Create the worker and scheduler threads.
    //
    pthread_t workerThread;
    if (pthread_create(&workerThread, 0, Worker, 0) != 0)
    {
        std::cerr << "Unable to create worker thread" << std::endl;
        return 1;
    }
    pthread_t schedulerThread;
    if (pthread_create(&schedulerThread, 0, Scheduler, 0) != 0)
    {
        std::cerr << "Unable to create scheduler thread" << std::endl;
        return 1;
    }

    pthread_join(workerThread, 0);
    pthread_join(schedulerThread, 0);

    return (IsError) ? 1 : 0;
}

static void* Worker(void*)
{
    WorkerTid = GetTid();
    if (!SetPriority(WorkerTid, PRIORITY_HIGH) || !SetAffinity(WorkerTid)) IsError = true;

    pthread_mutex_lock(&Lock);
    WorkerIsInitialized = true;
    pthread_cond_signal(&WorkerInitialized);
    pthread_mutex_unlock(&Lock);

    // Execute the work loop via a volatile pointer to prevent the compiler
    // from inlining.  The Pin tool instruments DoWorkInstrumentedWithPin(),
    // so we don't want its body to be inlined.
    //
    volatile VOIDFUNPTR doWork = DoWorkInstrumentedWithPin;
    doWork();
    return 0;
}

static void* Scheduler(void*)
{
    if (!SetPriority(GetTid(), PRIORITY_HIGH) || !SetAffinity(GetTid())) IsError = true;

    // Wait for the worker to initialize itself.
    //
    pthread_mutex_lock(&Lock);
    while (!WorkerIsInitialized)
        pthread_cond_wait(&WorkerInitialized, &Lock);
    pthread_mutex_unlock(&Lock);

    // This loop tries to lower the priority of the worker while it holds
    // the PIN_LOCK.
    //
    volatile VOIDFUNPTR doGetLockWithPin = DoGetLockWithPin;
    for (unsigned long i = 0; i < NUM_SCHEDULES && !IsError; i++)
    {
        // Lower the priority, then try to acquire the PIN_LOCK.  We want
        // to attempt to acuire the lock here while the worker has the
        // lock and is running at low priority.
        //
        if (!SetPriority(WorkerTid, PRIORITY_LOW)) IsError = true;
        doGetLockWithPin();

        if ((i % (NUM_SCHEDULES / 10)) == 0) std::cout << "Iterations: " << std::dec << i << std::endl;

        // Raise the worker priority and yield the processor to it.  Let the
        // worker start running again before the next attempt.
        //
        if (!SetPriority(WorkerTid, PRIORITY_HIGH)) IsError = true;
        sched_yield();
    }

    Done = true;
    return 0;
}

extern "C" void DoWorkInstrumentedWithPin()
{
    // This is the worker loop that is instrumented by Pin.  This loop
    // continually acquires and releases a PIN_LOCK.
    //
    while (!Done)
        ;
}

extern "C" void DoGetLockWithPin()
{
    // This is executed by the scheduler after it has lowered the priority of
    // the worker.  Pin also tries to acquire the PIN_LOCK here.
}

extern "C" void TellPinNotSupported()
{
    // The Pin tool instruments this function, so that it is informed if the
    // test is not run because the O/S doesn't support real-time scheduling.
}

// Change the priority of a thread to "high" or "low".
//
static bool SetPriority(pid_t tid, PRIORITY priority)
{
    struct sched_param param;
    param.sched_priority = (priority == PRIORITY_HIGH) ? PriorityHigh : PriorityLow;

    if (sched_setscheduler(tid, SCHED_RR, &param) != 0)
    {
        pthread_mutex_lock(&ErrorLock);
        std::cerr << "Error from sched_setscheduler(0x" << std::hex << tid << "), errno=" << std::dec << errno << std::endl;
        pthread_mutex_unlock(&ErrorLock);
        return false;
    }
    return true;
}

// Constrain a thread so that it only runs on CPU #0.
//
static bool SetAffinity(pid_t tid)
{
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(0, &cpus);

    if (sched_setaffinity(tid, sizeof(cpus), &cpus) != 0)
    {
        pthread_mutex_lock(&ErrorLock);
        std::cerr << "Error from sched_setaffinity(0x" << std::hex << tid << "), errno=" << std::dec << errno << std::endl;
        pthread_mutex_unlock(&ErrorLock);
        return false;
    }
    return true;
}

// Check if the O/S supports real-time scheduling.
//
static bool CheckSupported()
{
    pid_t tid = GetTid();
    if (tid == -1)
    {
        std::cout << "Platform doesn't support SYS_gettid.\n";
        return false;
    }

    struct sched_param param;
    param.sched_priority = 2;
    if (sched_setscheduler(tid, SCHED_RR, &param) != 0)
    {
        std::cout << "Cannot set SCHED_RR priority level 2.  Do one of the following to fix:\n";
        std::cout << "  1) Run this test manually as root.\n";
        std::cout << "  2) Edit the \"/etc/security/limits.conf\" file and raise the \"rtprio\"\n";
        std::cout << "     limit to at least 2.\n";
        return false;
    }

    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(0, &cpus);

    if (sched_setaffinity(tid, sizeof(cpus), &cpus) != 0)
    {
        std::cout << "Cannot set CPU affinity to CPU 0.\n";
        return false;
    }

    return true;
}

// Get the O/S thread ID of the caller.
//
static pid_t GetTid()
{
    pid_t tid = syscall(SYS_gettid);
    return tid;
}

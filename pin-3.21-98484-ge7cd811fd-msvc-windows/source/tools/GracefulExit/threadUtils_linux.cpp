/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include "threadUtils.h"

/**************************************************
 * Type definitions                               *
 **************************************************/
typedef void* (*startfunc)(void*);

/**************************************************
 * Global variables                               *
 **************************************************/
volatile int numOfThreads = 0;

/**************************************************
 * Global locks                                   *
 **************************************************/
pthread_mutex_t printLock;      // This lock is used for synchronizing prints.
pthread_mutex_t numThreadsLock; // This lock is used for synchronizing access to numOfThreads.

/**************************************************
 * Static functions declaration                   *
 **************************************************/
void GetLock(pthread_mutex_t* mutex);
void ReleaseLock(pthread_mutex_t* mutex);
static void TimeoutHandler(int a);

/**************************************************
 * External functions implementation              *
 **************************************************/
unsigned int GetTid() { return (unsigned int)syscall(__NR_gettid); }

void InitLocks()
{
    if (pthread_mutex_init(&printLock, NULL))
    {
        ErrorExit(RES_INIT_FAILED);
    }
    if (pthread_mutex_init(&numThreadsLock, NULL))
    {
        ErrorExit(RES_INIT_FAILED);
    }
}

bool CreateNewThread(TidType* tid, void* func, void* info) { return (pthread_create(tid, NULL, (startfunc)func, info) == 0); }

void CancelThread(TidType tid)
{
    if (pthread_cancel(tid) != 0)
    {
        ErrorExit(RES_CANCEL_FAILED);
    }
}

void WaitForThread(TidType tid)
{
    if (pthread_join(tid, NULL) != 0)
    {
        ErrorExit(RES_JOIN_FAILED);
    }
}

void ThreadExit() { pthread_exit(NULL); }

void IncThreads()
{
    GetLock(&numThreadsLock);
    ++numOfThreads;
    ReleaseLock(&numThreadsLock);
}

void DecThreads()
{
    GetLock(&numThreadsLock);
    --numOfThreads;
    ReleaseLock(&numThreadsLock);
}

int NumOfThreads() { return numOfThreads; }

void Print(const string& str)
{
    GetLock(&printLock);
    fprintf(stderr, "APP:  <%d> %s\n", GetTid(), str.c_str());
    ReleaseLock(&printLock);
}

void ErrorExit(Results res)
{
    GetLock(&printLock);
    fprintf(stderr, "APP ERROR <%d>: %s\n", GetTid(), errorStrings[res].c_str());
    ReleaseLock(&printLock);
    exit(res);
}

void DoSleep(unsigned int seconds) { sleep(seconds); }

void EnterSafeCancellationPoint()
{
    while (1)
    {
        sleep(1);
    }
}

void SetTimeout()
{
    struct sigaction sigact_timeout;
    sigact_timeout.sa_handler = TimeoutHandler;
    sigact_timeout.sa_flags   = 0;

    if (sigaction(SIGALRM, &sigact_timeout, 0) == -1)
    {
        Print("Unable to set up timeout handler\n");
        ErrorExit(RES_UNEXPECTED_EXIT);
    }
    alarm(TIMEOUT);
}

/**************************************************
 * Static functions implementation                *
 **************************************************/
void GetLock(pthread_mutex_t* mutex)
{
    if (pthread_mutex_lock(mutex) != 0)
    {
        ErrorExit(RES_LOCK_FAILED);
    }
}

void ReleaseLock(pthread_mutex_t* mutex)
{
    if (pthread_mutex_unlock(mutex) != 0)
    {
        ErrorExit(RES_UNLOCK_FAILED);
    }
}

void DoYield() { sched_yield(); }

void TimeoutHandler(int a)
{
    Print("Application is running more than 10 minutes. It might be hanged. Killing it");
    ErrorExit(RES_EXIT_TIMEOUT);
}

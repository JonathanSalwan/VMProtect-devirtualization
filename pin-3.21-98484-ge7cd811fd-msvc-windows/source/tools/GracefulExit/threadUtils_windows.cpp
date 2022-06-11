/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <map>
#include <cstdio>
#include <cstdlib>
#include "threadUtils.h"

using std::map;

/**************************************************
 * Global variables                               *
 **************************************************/
static volatile int numOfThreads = 0;
static map< TidType, HANDLE > handles;

/**************************************************
 * Global locks                                   *
 **************************************************/
static HANDLE printLock;      // This mutex lock is used for synchronizing prints.
static HANDLE numThreadsLock; // This mutex lock is used for synchronizing access to numOfThreads.

/**************************************************
 * Static functions declaration                   *
 **************************************************/
static void GetLock(HANDLE* thelock);
static void ReleaseLock(HANDLE* thelock);
static void ErrorExitUnlocked(Results res);

static HANDLE cancellationPoint;

/**************************************************
 * External functions implementation              *
 **************************************************/
unsigned int GetTid() { return (unsigned int)GetCurrentThreadId(); }

void InitLocks()
{
    printLock         = CreateMutex(NULL, FALSE, NULL);
    numThreadsLock    = CreateMutex(NULL, FALSE, NULL);       // Create mutexes without initial ownership.
    cancellationPoint = CreateEvent(NULL, TRUE, FALSE, NULL); // manual reset event
}

bool CreateNewThread(TidType* tid, void* func, void* info)
{
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, info, 0, tid);
    if (hThread == NULL) return false;
    handles[*tid] = hThread;
    return true;
}

void CancelThread(TidType tid)
{
    if (TerminateThread(handles[tid], 0) == 0)
    {
        ErrorExit(RES_CANCEL_FAILED);
    }
}

void WaitForThread(TidType tid)
{
    if (WaitForSingleObject(handles[tid], INFINITE) == WAIT_FAILED)
    {
        ErrorExit(RES_JOIN_FAILED);
    }
}

void TimeoutThreadFunc(void* info)
{
    DoSleep(TIMEOUT);

    // Should never get here, only if timeout occurred
    Print("Got timeout, exiting test with an error");
    ErrorExit(RES_EXIT_TIMEOUT);
}

void SetTimeout()
{
    TidType tid;
    if (!CreateNewThread(&tid, (void*)TimeoutThreadFunc, NULL))
    {
        Print("Could not open a timeout thread");
        ErrorExit(RES_CREATE_FAILED);
    }
}

void ThreadExit() { ExitThread(0); }

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
    fflush(stderr);
    ReleaseLock(&printLock);
}

void ErrorExit(Results res)
{
    GetLock(&printLock);
    fprintf(stderr, "APP ERROR <%d>: %s\n", GetTid(), errorStrings[res].c_str());
    fflush(stderr);
    ReleaseLock(&printLock);
    exit(res);
}

void ErrorExitUnlocked(Results res)
{
    fflush(stderr);
    fprintf(stderr, "APP ERROR <%d>: %s\n", GetTid(), errorStrings[res].c_str());
    fflush(stderr);
    exit(res);
}

void DoSleep(unsigned int seconds) { Sleep(seconds * 1000); }

void DoYield() { Yield(); }

void EnterSafeCancellationPoint() { WaitForSingleObject(cancellationPoint, INFINITE); }

/**************************************************
 * Static functions implementation                *
 **************************************************/
void GetLock(HANDLE* thelock)
{
    DWORD status = WaitForSingleObject(*thelock, INFINITE);
    if (status == WAIT_OBJECT_0) return; // Success.
    // Do not proceed if status is either WAIT_ABANDONED (thread that owned the mutex was terminated,
    // current thread takes ownership, but we don't want to recover), or WAIT_FAILED (any other failure).
    ErrorExitUnlocked(RES_LOCK_FAILED);
}

void ReleaseLock(HANDLE* thelock)
{
    if (ReleaseMutex(*thelock)) return; // Success.
    ErrorExitUnlocked(RES_UNLOCK_FAILED);
}

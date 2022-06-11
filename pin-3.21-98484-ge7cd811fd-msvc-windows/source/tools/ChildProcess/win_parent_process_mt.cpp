/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// MT Application that creates new processes, from multiple threads simultaneously

#include <Windows.h>
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

// The number 53 stresses the system up to the point of crashing due to resource exhaustion.
const LONG numthreads = 53;

LONG volatile numThreadsArrived = 0;

HANDLE threadReachedSynchPointSemHandle[numthreads];
CRITICAL_SECTION critSec;

void MutexWriteToStdout(char* msg)
{
    EnterCriticalSection(&critSec);

    printf(msg);

    fflush(stdout);

    LeaveCriticalSection(&critSec);
}

void MutexWriteToStdoutWithValue(char* msg, DWORD value)
{
    EnterCriticalSection(&critSec);

    printf(msg, value);

    fflush(stdout);

    LeaveCriticalSection(&critSec);
}

//Wait for a process completion
//Verify it returned the expected exit code
bool WaitAndVerify(HANDLE process)
{
    if (WaitForSingleObject(process, INFINITE) == WAIT_FAILED)
    {
        MutexWriteToStdout("WaitForSingleObject failed\n");
        return FALSE;
    }
    DWORD processExitCode;
    if (GetExitCodeProcess(process, &processExitCode) == FALSE)
    {
        MutexWriteToStdout("GetExitCodeProcess Failed\n");
        return FALSE;
    }
    if (processExitCode != 0)
    {
        MutexWriteToStdoutWithValue("Got unexpected exit code %x\n", processExitCode);
        return FALSE;
    }
    return TRUE;
}

// Cause all threads to wait here until all the threads arrive here
void AllThreadSynchPoint(LONG threadNum)
{
    LONG myNumThreadsArrived, i;

    myNumThreadsArrived = InterlockedIncrement(&numThreadsArrived);
    if (myNumThreadsArrived == numthreads)
    {                          // all threads have arrived
                               // wake the others
        numThreadsArrived = 0; // reset for next thread synch point
        for (i = 0; i < numthreads; i++)
        {
            if (i != threadNum)
            {
                ReleaseSemaphore(threadReachedSynchPointSemHandle[i], 1, NULL);
            }
        }
    }
    else
    { // thread waits here till all other threads arrive
        WaitForSingleObject(threadReachedSynchPointSemHandle[threadNum], INFINITE);
    }
}

DWORD WINAPI CreateTestProcesses(LPVOID threadNumPtr)
{
    char cmd[] = "win_child_process.exe \"param1 param2\" param3 5000";
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    LONG threadNum = (LONG)threadNumPtr;
    BOOL ok;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    memset(&pi, 0, sizeof(pi));

    AllThreadSynchPoint(threadNum); // best attempt to get the threads to call CreateProcess
                                    // simultaneously
    if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
    {
        DWORD lastError = GetLastError();
        MutexWriteToStdoutWithValue("Couldn't create child process, code %x\n", lastError);
        ok = FALSE;
    }
    else
    {
        CloseHandle(pi.hThread);
        ok = WaitAndVerify(pi.hProcess);
        CloseHandle(pi.hProcess);
    }
    AllThreadSynchPoint(threadNum);
    if (ok)
    {
        MutexWriteToStdout("First Process was created successfully!\n");
    }
    else
    {
        MutexWriteToStdoutWithValue("First Process was not created successfully in thread %d\n", threadNum);
    }

    //Create suspended

    AllThreadSynchPoint(threadNum);
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    memset(&pi, 0, sizeof(pi));
    if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &si, &pi))
    {
        DWORD lastError = GetLastError();
        MutexWriteToStdoutWithValue("Couldn't create child process suspended, code %x \n", lastError);
        ok = FALSE;
    }
    else
    {
        ResumeThread(pi.hThread);
        CloseHandle(pi.hThread);

        ok = WaitAndVerify(pi.hProcess);
        CloseHandle(pi.hProcess);
    }

    AllThreadSynchPoint(threadNum);
    if (ok)
    {
        MutexWriteToStdout("Second Process was created successfully!\n");
    }
    else
    {
        MutexWriteToStdoutWithValue("Second Process was not created successfully in thread %d\n", threadNum);
    }

    //Create process as user

    AllThreadSynchPoint(threadNum);
    HANDLE tokenHandle;
    BOOL res = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &tokenHandle);
    if (!res)
    {
        MutexWriteToStdout("Couldn't open process token\n");
        tokenHandle = NULL;
    }

    AllThreadSynchPoint(threadNum);
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    memset(&pi, 0, sizeof(pi));
    if (!CreateProcessAsUser(tokenHandle, NULL, cmd, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &si, &pi))
    {
        DWORD lastError = GetLastError();
        CloseHandle(tokenHandle);
        MutexWriteToStdoutWithValue("Couldn't create child process as user, code %x \n", lastError);
        ok = FALSE;
    }
    else
    {
        CloseHandle(tokenHandle);
        ResumeThread(pi.hThread);
        CloseHandle(pi.hThread);

        ok = WaitAndVerify(pi.hProcess);
        CloseHandle(pi.hProcess);
    }

    AllThreadSynchPoint(threadNum);
    if (ok)
    {
        MutexWriteToStdout("Third Process was created successfully!\n");
    }
    else
    {
        MutexWriteToStdoutWithValue("Third Process was not created successfully in thread %d\n", threadNum);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    LONG i;
    HANDLE threadHandles[numthreads];

    if (numthreads > MAXIMUM_WAIT_OBJECTS)
    {
        cout << "exceeded limit of waitable objects\n";
        return 2;
    }

    InitializeCriticalSection(&critSec);

    for (i = 0; i < numthreads; i++)
    {
        threadReachedSynchPointSemHandle[i] = CreateSemaphore(NULL, 0, 1, NULL);
        if (threadReachedSynchPointSemHandle[i] == NULL)
        {
            cout << "failed to create threadReachedSynchPointSemHandle\n";
            return -1;
        }
    }

    for (i = 0; i < numthreads; i++)
    {
        threadHandles[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CreateTestProcesses, (LPVOID)i, 0, NULL);
        if (threadHandles[i] == NULL)
        {
            cout << "failed to create thread\n";
            return -2;
        }
    }
    // Wait until all threads terminated
    WaitForMultipleObjects((DWORD)numthreads, threadHandles, TRUE, INFINITE);

    DeleteCriticalSection(&critSec);

    Sleep(1000);

    return 0;
}

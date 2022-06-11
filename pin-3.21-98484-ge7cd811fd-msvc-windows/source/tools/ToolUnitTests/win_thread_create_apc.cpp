/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <Windows.h>
#include <stdio.h>

// Thread creation - exercise windows APC mechanism

extern "C" __declspec(dllexport) int ThreadRoutine() { return 0; }

bool ThreadCreation()
{
    const unsigned long num_threads     = 64;
    static HANDLE aThreads[num_threads] = {0};
    unsigned long slot                  = 0;
    unsigned long thread_id             = 0;
    unsigned long cnt_th                = 0;

    for (cnt_th = 0; cnt_th < num_threads; cnt_th++)
    {
        aThreads[cnt_th] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadRoutine, 0, 0, (LPDWORD)&thread_id);
        if (aThreads[cnt_th] == NULL)
        {
            fprintf(stderr, "CreateThread failed with GetLastError = %X \n", GetLastError());
            return false;
        }
    }

    fprintf(stderr, "created %d threads \n", num_threads);
    // Calling WaitForMultipleObjects with bWaitAll=FALSE will return for each terminated thread,
    // returning the index of the signalled object.
    // It allows the application to call CloseHandle for each terminated thread while other threads are still running.
    while (cnt_th > 0)
    {
        bool waitAll = FALSE;
        slot         = WaitForMultipleObjects(cnt_th, aThreads, waitAll, INFINITE);
        if (slot >= cnt_th)
        {
            fprintf(stderr, "WaitForMultipleObjects returned %X with GetLastError = %X\n", slot, GetLastError());
            return false;
        }
        CloseHandle(aThreads[slot]);
        aThreads[slot] = aThreads[cnt_th - 1];
        cnt_th--;
    }
    fprintf(stderr, "all %d threads terminated\n", num_threads);
    fflush(stderr);
    return true;
}

extern "C" __declspec(dllimport) void Nothing();

int main()
{
    Nothing();
    return ThreadCreation() ? 0 : 1;
}

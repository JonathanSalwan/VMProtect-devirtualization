/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <Windows.h>
#include <stdio.h>

// Combination of thread creation and load library

extern "C" __declspec(dllexport) int ThreadRoutine()
{
    // use the loader lock
    for (int i = 0; i < 100000; i++)
    {
        GetModuleHandle("kernel32.dll");
    }
    return 0;
}

bool ThreadCreateAndLoadLibrary()
{
    //thread creation
    const unsigned long num_threads     = 1;
    static HANDLE aThreads[num_threads] = {0};
    unsigned long slot                  = 0;
    unsigned long thread_id             = 0;
    unsigned long cnt_th                = 0;
    unsigned long thread_ret            = 0;

    for (cnt_th = 0; cnt_th < num_threads; cnt_th++)
    {
        aThreads[cnt_th] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadRoutine, 0, 0, (LPDWORD)&thread_id);
    }
    fprintf(stderr, "created %d threads \n", num_threads);

    //load library
    HMODULE hker32;
    int iterations = 10;
    for (int i = 0; i < iterations; i++)
    {
        hker32 = LoadLibrary("kernel32.dll");
    }
    fprintf(stderr, "loaded kernel32 %d times \n", iterations);

    //thread termination
    while (cnt_th > 0)
    {
        slot = WaitForMultipleObjects(cnt_th, aThreads, FALSE, INFINITE);
        GetExitCodeThread(aThreads[slot], &thread_ret);
        CloseHandle(aThreads[slot]);
        aThreads[slot] = aThreads[cnt_th - 1];
        cnt_th--;
    }
    fprintf(stderr, "all %d threads terminated\n", num_threads);
    fflush(stderr);
    return true;
}

int main()
{
    ThreadCreateAndLoadLibrary();
    return 0;
}
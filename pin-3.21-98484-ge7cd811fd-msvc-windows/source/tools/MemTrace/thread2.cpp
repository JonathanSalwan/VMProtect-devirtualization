/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
#include "../Utils/threadlib.h"

#if defined(TARGET_WINDOWS)
#include <windows.h>
#define EXPORT_CSYM extern "C" __declspec(dllexport)

#else
#define EXPORT_CSYM extern "C"

#endif

int a[100000];
int n                                      = 10;
long numthreads                            = 16;
EXPORT_CSYM unsigned int numthreadsStarted = 0;

extern "C" void AtomicIncrement();

EXPORT_CSYM void DoWork()
{
    int i, j;

    for (j = 0; j < 1000; j++)
    {
        for (i = 0; i < n; i++)
        {
            a[i] = 1;
        }
    }
}

EXPORT_CSYM void WaitForAllThreadsStarted()
{
    AtomicIncrement(); // atomically increments numthreadsStarted
    while (numthreadsStarted != numthreads)
    {
    }
}

EXPORT_CSYM void* ThreadStart(void* arg)
{
    int i;
    // no thread starts the work loop until all threads are in the ThreadStart function
    WaitForAllThreadsStarted();
    for (i = 0; i < 100; i++)
    {
        DoWork();
    }
    return (NULL);
}

THREAD_HANDLE threads[MAXTHREADS];

EXPORT_CSYM int main(int argc, char* argv[])
{
    int i, j;

    for (i = 0; i < numthreads; i++)
    {
        CreateOneThread(&threads[i], ThreadStart, 0);
    }

    for (i = 0; i < numthreads; i++)
    {
        BOOL success;
        success = JoinOneThread(threads[i]);
    }

    return 0;
}

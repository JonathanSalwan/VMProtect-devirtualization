/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
#include "../Utils/threadlib.h"

/* TestSegmentedEA is assembly code that contains various accesses to segmented memory
   in each of the possible addressing modes
*/
#if defined(__cplusplus)
extern "C"
#endif
    void
    TestSegmentedEA();

int a[100000];
int n = 10;

#ifdef TARGET_WINDOWS
extern __declspec(dllexport) __declspec(noinline)
#endif
    void* longfun(void* arg)
{
    int i, j;

    for (j = 0; j < 1000; j++)
    {
        for (i = 0; i < n; i++)
        {
            a[i] = 1;
        }
    }
    TestSegmentedEA();
    return 0;
}

#ifdef TARGET_WINDOWS
extern __declspec(dllexport) __declspec(noinline)
#endif
    void* shortfun(void* arg)
{
    a[1] = 1;
    TestSegmentedEA();
    return 0;
}

THREAD_HANDLE threads[MAXTHREADS];

int main(int argc, char* argv[])
{
    int numthreads = 0;
    int i;

    TestSegmentedEA();
    numthreads = 30;
    assert(numthreads < MAXTHREADS);

    for (i = 0; i < numthreads; i++)
    {
        printf("Creating thread %d\n", i);
        fflush(stdout);
        if (i % 2 == 0)
            CreateOneThread(&threads[i], longfun, 0);
        else
            CreateOneThread(&threads[i], shortfun, 0);
    }
    for (i = 0; i < numthreads; i++)
    {
        BOOL success;
        success = JoinOneThread(threads[i]);
        if (!success)
        {
            fprintf(stdout, "JoinOneThread failed\n");
            fflush(stdout);
        }
    }

    printf("All threads joined\n");
    fflush(stdout);

    return 0;
}

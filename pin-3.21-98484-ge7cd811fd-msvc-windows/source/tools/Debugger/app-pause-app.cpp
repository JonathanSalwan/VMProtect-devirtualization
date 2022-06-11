/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef TARGET_WINDOWS
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../Utils/threadlib.h"
#include "atomic.hpp"

#define NUM_TH 3
#define LOOPS 200000000

volatile size_t numThreads = 0;
size_t retval[NUM_TH + 1];
THREAD_HANDLE threads[NUM_TH + 1];

void* compute(void* ptr)
{
    size_t th_id = (size_t)ptr;
    size_t i = 0, newnum = 0;

    printf("Thread start %u\n", (unsigned int)th_id);
    fflush(stdout);

    ATOMIC::OPS::Increment< size_t >(&numThreads, 1);
    while (numThreads < NUM_TH)
    {
        DelayCurrentThread(1);
    }

    for (i = 0; i < LOOPS; i++)
    {
        newnum += (i + newnum);
    }

    printf("Thread end %u\n", (unsigned int)th_id);
    fflush(stdout);
    retval[th_id] = i;
    return (void*)newnum;
}

int main()
{
    for (size_t i = 1; i <= NUM_TH; i++)
    {
        CreateOneThread(&threads[i], compute, (void*)i);
    }

    bool ok = true;
    for (size_t i = 1; i <= NUM_TH; i++)
    {
        ok = ok && JoinOneThread(threads[i]);
        ok = ok && (retval[i] == LOOPS);
    }

    exit(ok ? 0 : 1);
}

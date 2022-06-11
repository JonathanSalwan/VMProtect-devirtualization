/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
#include "../Utils/threadlib.h"

int a[100000];
int n = 10;

void* hello(void* arg)
{
    int i, j;

    for (j = 0; j < 1000; j++)
    {
        for (i = 0; i < n; i++)
        {
            a[i] = 1;
        }
    }

    return 0;
}

THREAD_HANDLE threads[MAXTHREADS];

int main(int argc, char* argv[])
{
    int numthreads = 0;
    int i, j;

    numthreads = 4;

    for (j = 0; j < 100; j++)
    {
        for (i = 0; i < numthreads; i++)
        {
            CreateOneThread(&threads[i], hello, 0);
        }

        for (i = 0; i < numthreads; i++)
        {
            BOOL success;
            success = JoinOneThread(threads[i]);
        }
    }

    return 0;
}

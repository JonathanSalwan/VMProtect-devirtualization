/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sched.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define MAXTHREADS 1000

int data[MAXTHREADS];

void* start(void* arg)
{
    int i = 0;
    for (i = 0; i < 1000; i++)
    {
        void* h = malloc(13);
        if (h) free(h);
    }
    return 0;
}

int main(int argc, char* argv[])
{
    pthread_t threads[MAXTHREADS];
    int i;
    int numthreads = 20;

    printf("Creating %d threads\n", numthreads);

    for (i = 0; i < numthreads; i++)
    {
        pthread_create(&threads[i], 0, start, 0);
    }

    for (i = 0; i < numthreads; i++)
    {
        pthread_join(threads[i], 0);
    }
    printf("All threads joined\n");

    return 0;
}

/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>

#define EXPORT_SYM extern "C"

#define NTHREADS 20

volatile bool loop;

EXPORT_SYM bool AfterAttach2();

bool AfterAttach2()
{
    // Pin sets an anslysis function here to notify the application when Pin attaches to it.
    return false;
}

enum ExitType
{
    RES_SUCCESS = 0, // 0
    RES_LOAD_FAILED, // 1
    RES_INVALID_ARGS // 2
};

void* thread_func(void* arg)
{
    while (loop)
    {
        sched_yield();
    }
    return 0;
}

/*
    Expected argv arguments:
    [1]	First image to load	
    [2]	Second image to load
*/
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Not enough arguments\n");
        fflush(stderr);
        exit(RES_INVALID_ARGS);
    }

    loop = true;

    pthread_t h[NTHREADS];
    int ret_val;
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        ret_val = pthread_create(&h[i], 0, thread_func, 0);
        if (ret_val)
        {
            perror("ERROR, pthread_create failed");
            exit(1);
        }
    }

    void* handle = dlopen(argv[1], RTLD_LAZY);
    if (!handle)
    {
        fprintf(stderr, " Failed to load: %s because: %s\n", argv[1], dlerror());
        fflush(stderr);
        exit(RES_LOAD_FAILED);
    }

    while (!AfterAttach2())
    {
        sleep(1);
    }

    handle = dlopen(argv[2], RTLD_LAZY);
    if (!handle)
    {
        fprintf(stderr, " Failed to load: %s because: %s\n", argv[2], dlerror());
        fflush(stderr);
        exit(RES_LOAD_FAILED);
    }

    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        pthread_join(h[i], 0);
    }

    return RES_SUCCESS;
}

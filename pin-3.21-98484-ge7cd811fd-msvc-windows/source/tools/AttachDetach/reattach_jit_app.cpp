/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Test detaching - reattach Pin on Linux
 *  The application creates threads in a loop. Each thread does some work and exits.
 *  The process in signaled by the pintool (by replacing AppShouldExit()) that it should exit.
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <math.h>
#include "../Utils/threadlib.h"

#define NTHREADS 20

volatile bool loop1, loop2;
void* thread_func(void* arg)
{
    do
    {
        void* space = malloc(300);
        //sleep(1);
        free(space);
    }
    while (loop2);

    return 0;
}

extern "C" int AppShouldExit() { return 0; }

typedef double (*DLL_FUNC)(double x);
typedef int (*DLL_INT_FUNC)(int x);

void* thread_dlopen_func(void* arg)
{
    double number          = 0.2;
    double calculatedValue = 0;
    do
    {
        void* handle = dlopen("libmy_dll.so", RTLD_LAZY);
        if (handle)
        {
            DLL_FUNC fptr = (DLL_FUNC)dlsym(handle, "my_dll_sin");
            calculatedValue += (*fptr)(number);

            //sleep(1);
            dlclose(handle);
        }
        else
        {
            fprintf(stderr, "error opening my_dll.so, thread %d\n", GetTid());
            exit(-1);
        }
        number += 0.01;
    }
    while (loop1);

    return 0;
}

int main(int argc, char* argv[])
{
    const unsigned int numOfSeconds = 5 * 60; // allow 5 minutes
    unsigned int secondsRemaining   = numOfSeconds;
    for (; secondsRemaining && (!AppShouldExit()); --secondsRemaining)
    {
        loop1 = true;
        loop2 = true;
        pthread_t h[NTHREADS];

        pthread_create(&h[0], 0, thread_dlopen_func, 0);
        for (unsigned long i = 1; i < NTHREADS; i++)
        {
            pthread_create(&h[i], 0, thread_func, 0);
        }
        sleep(1);

        loop1 = false;
        loop2 = false;

        for (unsigned long i = 0; i < NTHREADS; i++)
        {
            pthread_join(h[i], 0);
        }
    }
    if (secondsRemaining == 0)
    {
        fprintf(stderr, "ERROR: The application ran for %u seconds without completion\n", numOfSeconds);
        return 1;
    }
    return 0;
}

/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Test detaching - reattach Pin on Linux
 *  The application creates threads in a loop. Each thread does some work and exits.
 *  The process in endless, expected to be stopped by tool.
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

#ifdef TARGET_LINUX

#define DLL_SUFFIX ".so"

#elif defined(TARGET_MAC)

#define DLL_SUFFIX ".dylib"

#endif

#define TLS_GET_GS_REG()                           \
    (                                              \
        {                                          \
            int __seg;                             \
            __asm("movw %%gs, %w0" : "=q"(__seg)); \
            __seg & 0xffff;                        \
        })

volatile bool loop1, loop2;
void* thread_func(void* arg)
{
    while (loop2)
    {
        void* space = malloc(300);
        //sleep(1);
        free(space);
    }
    return 0;
}

typedef double (*DLL_FUNC)(double x);
typedef int (*DLL_INT_FUNC)(int x);

void* thread_dlopen_func(void* arg)
{
    double number          = 0.2;
    double calculatedValue = 0;
    while (loop1)
    {
        void* handle = dlopen("libmy_dll" DLL_SUFFIX, RTLD_LAZY);
        if (handle)
        {
            DLL_FUNC fptr = (DLL_FUNC)dlsym(handle, "my_dll_sin");
            calculatedValue += (*fptr)(number);

            //sleep(1);
            dlclose(handle);
        }
        else
        {
            fprintf(stderr, "error opening my_dll" DLL_SUFFIX ", thread %ld\n", GetTid());
            exit(-1);
        }
        number += 0.01;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    unsigned long gs_val = TLS_GET_GS_REG();
    while (1)
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
    return 0;
}

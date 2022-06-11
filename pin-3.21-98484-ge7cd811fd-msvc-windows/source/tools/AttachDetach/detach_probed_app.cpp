/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Test detaching Pin from running process on a Unix compatible system
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>

#define NTHREADS 20

#ifdef TARGET_MAC
#define DLL_SUFFIX ".dylib"
#else
#define DLL_SUFFIX ".so"
#endif

extern "C" void TellPinToDetach(unsigned long* updateWhenReady) { return; }

volatile bool loop2 = true;
void* thread_func(void* arg)
{
    while (loop2)
    {
        void* space = malloc(300);
        sleep(1);
        free(space);
    }
    return 0;
}

volatile bool loop1 = true;
typedef double (*SIN_FUNC)(double x);

void* thread_dlopen_func(void* arg)
{
    double number = 0.2;
    while (loop1)
    {
        void* handle = dlopen("libm" DLL_SUFFIX, RTLD_LAZY);
        if (handle)
        {
            SIN_FUNC sin_fptr = (SIN_FUNC)dlsym(handle, "sin");
            if (sin_fptr)
            {
                double val = (*sin_fptr)(number);
                printf("val = %.4f\n", val);
            }
            sleep(2);
            dlclose(handle);
        }
        number += 0.01;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    pthread_t h[NTHREADS];

    pthread_create(&h[0], 0, thread_dlopen_func, 0);
    for (unsigned long i = 1; i < NTHREADS; i++)
    {
        pthread_create(&h[i], 0, thread_func, 0);
    }

    unsigned long pinDetached = false;
    TellPinToDetach(&pinDetached);

    while (!pinDetached)
    {
        sleep(2);
    }

    loop1 = false;
    loop2 = false;

    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        pthread_join(h[i], 0);
    }
    printf("All threads exited. The test PASSED\n");
    return 0;
}

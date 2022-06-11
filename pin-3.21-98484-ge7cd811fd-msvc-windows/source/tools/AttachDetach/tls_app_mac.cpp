/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * We test two aspects:
   - tls value before and after PIN_Detach()
   - creation new threads while PIN is detached from application

 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/utsname.h>

#define NTHREADS 4

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned int numOfThreadsReadyForDetach = 0;
unsigned long pinDetached               = 0;

extern "C" void TellPinToDetach(unsigned long* updateWhenReady) { return; }

// Get TLS value
void* ReadTLSBase()
{
    void* tlsBase;
#if defined(TARGET_IA32)
    asm("mov %%gs:0, %%eax\n"
        "mov %%eax, %0"
        : "=r"(tlsBase)
        :
        : "eax");
#else
    asm("mov %%gs:0, %%rax\n"
        "mov %%rax, %0"
        : "=r"(tlsBase)
        :
        : "rax");
#endif
    return tlsBase;
}
/*
 * Compare TLS_BASE values before and after detach.
 */
void* thread_func(void* arg)
{
    unsigned long thread_no = (unsigned long)arg + 1;

    void* tlsBase = 0;
    tlsBase       = ReadTLSBase();
    pthread_mutex_lock(&mutex);
    numOfThreadsReadyForDetach++;
    pthread_mutex_unlock(&mutex);

    while (!pinDetached)
    {
        usleep(100000); //0.1 seconds
    }

    void* tlsBaseAfterDetach = 0;
    tlsBaseAfterDetach       = ReadTLSBase();
    if (tlsBase != tlsBaseAfterDetach)
    {
        fprintf(stderr, "ERROR in thread %lu: GTLSBASE before detach %p; after detach %p\n", thread_no, tlsBase,
                tlsBaseAfterDetach);
        return (void*)1;
    }
    else
    {
        fprintf(stderr, "tls base in thread %lu: %p\n", thread_no, tlsBase);
    }
    return 0;
}

int main(int argc, char* argv[])
{
    pthread_t h[NTHREADS];

    void* tlsBase = 0;
    tlsBase       = ReadTLSBase();
    fprintf(stderr, "tls base in main thread: %p\n", tlsBase);
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        pthread_create(&h[i], 0, thread_func, (void*)i);
    }

    /*
     * If the number of threads is big, some threads leave system call "clone"
     * while PIN is detached. This functionality is also tested here.
     */

    TellPinToDetach(&pinDetached);

    void* result[NTHREADS];
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        pthread_join(h[i], &(result[i]));
    }
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        if (result[i] != 0)
        {
            fprintf(stderr, "TEST FAILED\n");
            return -1;
        }
    }
    void* tlsBaseAfterDetach = 0;
    tlsBaseAfterDetach       = ReadTLSBase();
    if (tlsBase != tlsBaseAfterDetach)
    {
        fprintf(stderr, "ERROR in the main thread: TLS_BASE before detach %p; after detach %p\n", tlsBase, tlsBaseAfterDetach);
        return -1;
    }
    fprintf(stderr, "TEST PASSED\n");
    return 0;
}

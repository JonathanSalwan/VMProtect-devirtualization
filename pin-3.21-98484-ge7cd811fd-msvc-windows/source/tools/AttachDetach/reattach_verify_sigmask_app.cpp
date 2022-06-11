/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Test detaching - reattach Pin on Linux
 *  The application verifies that signal mask is preserved during attach - detach.
 */
#include <assert.h>
#include <stdio.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sched.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <stdint.h>
#include "../Utils/threadlib.h"

volatile unsigned int unblockedUsr1Tid = 0;
volatile unsigned int unblockedUsr2Tid = 0;
volatile bool usr1Tested               = false;
volatile bool usr2Tested               = false;

pthread_mutex_t mutex;

extern "C" int AppShouldExit() { return 0; }

void SigUsrHandler(int sig)
{
    pthread_mutex_lock(&mutex);

    if (sig == SIGUSR1)
    {
        if (unblockedUsr1Tid == GetTid())
        {
            usr1Tested = true;
        }
        else
        {
            fprintf(stderr, "The signal mask is incorrect, SIGUSR1 is caught by %d, expected %d\n", GetTid(), unblockedUsr1Tid);
            exit(-1);
        }
    }
    if (sig == SIGUSR2)
    {
        if (unblockedUsr2Tid == GetTid())
        {
            usr2Tested = true;
        }
        else
        {
            fprintf(stderr, "The signal mask is incorrect, SIGUSR2 is caught by %d, expected %d\n", GetTid(), unblockedUsr2Tid);
            exit(-1);
        }
    }
    pthread_mutex_unlock(&mutex);
}

sigset_t sigSet;

void BlockSignal(int sigNo)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sigNo);
    pthread_sigmask(SIG_BLOCK, &mask, 0);
}

void UnblockSignal(int sigNo)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sigNo);
    pthread_sigmask(SIG_UNBLOCK, &mask, 0);
}
void UnblockAllSignals()
{
    sigset_t mask;
    sigemptyset(&mask);
    pthread_sigmask(SIG_SETMASK, &mask, 0);
}

void* ThreadFunc(void* arg)
{
    unsigned int thread_no = *(unsigned int*)&arg;

    if (thread_no == 1)
    {
        unblockedUsr1Tid = GetTid();
        UnblockSignal(SIGUSR1);
    }
    else if (thread_no == 2)
    {
        unblockedUsr2Tid = GetTid();
        UnblockSignal(SIGUSR2);
    }

    while (!usr1Tested || !usr2Tested)
    {
        sleep(1);
    }
    return 0;
}

#define NUM_OF_THREADS 7
pthread_t threads[NUM_OF_THREADS];

int main(int argc, char* argv[])
{
    // Set the same signal handler for USR1 and USR2 signals
    signal(SIGUSR1, SigUsrHandler);
    signal(SIGUSR2, SigUsrHandler);

    // initialize a mutex that will be used by threads
    pthread_mutex_init(&mutex, 0);

    /* Block signals in all threads */
    BlockSignal(SIGUSR1);
    BlockSignal(SIGUSR2);
    /*****************/

    while (!AppShouldExit())
    {
        // launch threads
        usr1Tested = false;
        usr2Tested = false;
        for (uintptr_t i = 0; i < NUM_OF_THREADS; i++)
        {
            pthread_create(&threads[i], 0, ThreadFunc, (void*)i);
        }

        while ((unblockedUsr1Tid == 0) || (unblockedUsr2Tid == 0))
        {
            sched_yield();
        }

        while (!usr1Tested || !usr2Tested)
        {
            kill(getpid(), SIGUSR1);
            kill(getpid(), SIGUSR2);
        }

        for (unsigned int i = 0; i < NUM_OF_THREADS; i++)
        {
            pthread_join(threads[i], 0);
        }
        unblockedUsr1Tid = 0;
        unblockedUsr2Tid = 0;
    }

    return 0;
}

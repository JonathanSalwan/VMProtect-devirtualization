/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This test checks that after Pin detaches from the application :
 * - the signal mask was restored correctly
 * - the application handlers were restored and they are called
 *
 * This test also check this case:
 * - Blocking some signal on all threads.
 *   Sending a signal (will be a pending signal).
 *   Detach Pin.
 *   Unblock signal in a specific thread.
 *   Make sure application received that signal in the thread mentioned above after the detach completed.
 */
#include <assert.h>
#include <mach/mach.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include "../Utils/threadlib.h"

#define NTHREADS 2

volatile unsigned int unblockedUsr1Tid = 0;
volatile unsigned int unblockedUsr2Tid = 0;
volatile unsigned int unblockedALRMTid = 0;
pthread_mutex_t mutex                  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sigHandlerMutex        = PTHREAD_MUTEX_INITIALIZER;

unsigned long pinDetached = 0;

extern "C" void TellPinToDetach(unsigned long* updateWhenReady)
{
    // Tool does this
    //*updateWhenReady=1;

    return;
}

// True when the correct signal was handled by the correct thread
static bool usr1Tested = false;
static bool usr2Tested = false;
static bool aLRMTested = false;

void signal_handler(int signum, siginfo_t* siginfo, void* _uctxt)
{
    pthread_mutex_lock(&sigHandlerMutex);

    // Verifying that the signal was received in the correct thread
    // (Signals are sent after detach)

    printf("Signal received: ");
    if (signum == SIGUSR1)
    {
        printf("SIGUSR1, in thread: %lu\n", GetTid());
        if (unblockedUsr1Tid == GetTid())
        {
            usr1Tested = true;
        }
        else
        {
            fprintf(stderr, "The signal mask is incorrect, SIGUSR1 is caught by %ld, expected %d\n", GetTid(), unblockedUsr1Tid);
            exit(-1);
        }
    }
    if (signum == SIGUSR2)
    {
        printf("SIGUSR2, in thread: %lu\n", GetTid());
        if (unblockedUsr2Tid == GetTid())
        {
            usr2Tested = true;
        }
        else
        {
            fprintf(stderr, "The signal mask is incorrect, SIGUSR2 is caught by %ld, expected %d\n", GetTid(), unblockedUsr2Tid);
            exit(-1);
        }
    }
    if (signum == SIGALRM)
    {
        printf("SIGALRM, in thread: %lu\n", GetTid());
        // We sent this signal before detach began, when it was blocked in all threads (pending)
        // We unblocked it after detach, therefore we expect to receive it after Pin detached.
        assert(pinDetached);

        if (unblockedALRMTid == GetTid())
        {
            aLRMTested = true;
        }
        else
        {
            fprintf(stderr, "The signal mask is incorrect, SIGALRM is caught by %ld, expected %d\n", GetTid(), unblockedALRMTid);
            exit(-1);
        }
    }
    if (usr1Tested && usr2Tested && aLRMTested)
    {
        printf("Signal handlers were changed to application ones after detach as expected\n");
    }
    pthread_mutex_unlock(&sigHandlerMutex);
}

/*
 *
 */
void* thread_func(void* arg)
{
    unsigned long thread_no = (unsigned long)arg + 1;
    sigset_t sigMask;
    sigset_t sigMaskAfterDeatch;
    bool* signalTested = NULL;
    int ret;

    // retrieve signal mask
    ret = pthread_sigmask(SIG_SETMASK, NULL, &sigMask);
    assert(0 == ret);

    // sanity check
    ret = sigismember(&sigMask, SIGUSR1);
    assert(1 == ret);
    ret = sigismember(&sigMask, SIGUSR2);
    assert(1 == ret);

    if (thread_no == 1)
    {
        unblockedUsr1Tid = GetTid();
        signalTested     = &usr1Tested;

        // Unblocking SIGUSR1
        sigemptyset(&sigMask);
        sigaddset(&sigMask, SIGUSR1);
        int ret = pthread_sigmask(SIG_UNBLOCK, &sigMask, NULL);
        assert(0 == ret);

        // retrieve signal mask
        ret = pthread_sigmask(SIG_UNBLOCK, NULL, &sigMask);
        assert(0 == ret);

        // sanity check
        ret = sigismember(&sigMask, SIGUSR1);
        assert(0 == ret);
        ret = sigismember(&sigMask, SIGUSR2);
        assert(1 == ret);
    }
    if (thread_no == 2)
    {
        unblockedUsr2Tid = GetTid();
        signalTested     = &usr2Tested;

        // Unblocking SIGUSR2
        sigemptyset(&sigMask);
        sigaddset(&sigMask, SIGUSR2);
        int ret = pthread_sigmask(SIG_UNBLOCK, &sigMask, NULL);
        assert(0 == ret);

        // retrieve signal mask
        ret = pthread_sigmask(SIG_UNBLOCK, NULL, &sigMask);
        assert(0 == ret);

        // sanity check
        ret = sigismember(&sigMask, SIGUSR2);
        assert(0 == ret);
        ret = sigismember(&sigMask, SIGUSR1);
        assert(1 == ret);
    }

    // Notify main thread it can continue
    pthread_mutex_unlock(&mutex);

    // Wait for detach to finish
    while (!pinDetached)
    {
        usleep(100000); //0.1 seconds
    }

    ret = pthread_sigmask(SIG_SETMASK, NULL, &sigMaskAfterDeatch);
    assert(0 == ret);

    if (sigMask != sigMaskAfterDeatch)
    {
        printf("ERROR: On Secondary thread, signal mask before detach: %x, Signal mask after detach: %x\n", sigMask,
               sigMaskAfterDeatch);
        exit(1);
    }

    // endless loop until signal will be received
    while (*signalTested == false)
    {
        sleep(1);
    }

    return 0;
}

int main()
{
    struct sigaction act;
    sigset_t fullMask, sigMask, sigMaskOld;
    int ret;

    bzero(&act, sizeof(act));
    act.sa_flags     = SA_SIGINFO;
    act.sa_sigaction = signal_handler;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGALRM, &act, NULL);

    // Block SIGUSR1 and SIGUSR2 on current thread
    sigemptyset(&sigMask);
    sigaddset(&sigMask, SIGUSR1);
    sigaddset(&sigMask, SIGUSR2);
    sigaddset(&sigMask, SIGALRM);
    ret = pthread_sigmask(SIG_BLOCK, &sigMask, NULL);
    assert(0 == ret);

    // retrieve signal mask
    ret = pthread_sigmask(SIG_SETMASK, NULL, &sigMaskOld);
    assert(0 == ret);

    // sanity check
    ret = sigismember(&sigMaskOld, SIGUSR1);
    assert(1 == ret);
    ret = sigismember(&sigMaskOld, SIGUSR2);
    assert(1 == ret);

    pthread_t h[NTHREADS];

    void* tlsBase = 0;
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        pthread_mutex_lock(&mutex);
        pthread_create(&h[i], 0, thread_func, (void*)i);
    }
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);

    // Sending SIGALRM (before detach), will be a pending since signal is blocked in all threads
    kill(getpid(), SIGALRM);

    printf("Request detach \n");
    TellPinToDetach(&pinDetached);

    // Wait for detach to finish
    while (!pinDetached)
    {
        usleep(100000); //0.1 seconds
    }

    printf("Detach completed\n");

    // retrieve signal mask
    ret = pthread_sigmask(SIG_SETMASK, NULL, &sigMask);
    assert(0 == ret);

    // Verify signal mask is the same after detach
    if (sigMask != sigMaskOld)
    {
        printf("ERROR: On main thread, signal mask before detach: %x, Signal mask after detach: %x\n", sigMaskOld, sigMask);
        exit(1);
    }

    unblockedALRMTid = GetTid();
    // Unblocking SIGALRM
    sigemptyset(&sigMask);
    sigaddset(&sigMask, SIGALRM);
    ret = pthread_sigmask(SIG_UNBLOCK, &sigMask, NULL);
    assert(0 == ret);

    // Sending these signals which will be handled on different threads
    kill(getpid(), SIGUSR1);
    kill(getpid(), SIGUSR2);

    // Wait for threads to exit
    // They will exit only when the above signals will handled,
    // this verifies that the signal handlers where changed to the ones of the application after the detach.

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

    assert(usr1Tested && usr2Tested && aLRMTested);

    printf("PASSED\n");
    return 0;
}

/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This application tests that a set of synchronous signals (Exceptions) which are raised (sent asynchronously)
 *  are received by the handler that was set for them.
 *
 *  When 1 is passes as an argument to the application it will also check a different scenarios where some of
 *  these signals are blocked when they are raised
 */

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <stdlib.h>

volatile int signalsReceived = 0; // Signal bitwise flag
#define ALL_SYNC_SIGNALS ((1 << SIGFPE) | (1 << SIGSEGV) | (1 << SIGILL) | (1 << SIGBUS) | (1 << SIGTRAP))

void Handler(int signum, siginfo_t* siginfo, void* _uctxt);

int CheckBlockedAysncExceptions()
{
    struct sigaction act;
    bzero(&act, sizeof(act));
    act.sa_flags     = SA_SIGINFO;
    act.sa_sigaction = Handler;

    sigaction(SIGFPE, &act, NULL);
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGTRAP, &act, NULL);

    int ret;
    sigset_t sigMask;

    // Block SIGFPE and SIGSEGV
    sigemptyset(&sigMask);
    sigaddset(&sigMask, SIGFPE);
    sigaddset(&sigMask, SIGSEGV);
    ret = pthread_sigmask(SIG_BLOCK, &sigMask, NULL);
    assert(0 == ret);

    // Raise some Exceptions (SIGFPE and SIGSEGV among them)
    raise(SIGFPE);
    raise(SIGSEGV);
    raise(SIGILL);
    raise(SIGBUS);

    // Verify SIGFPE and SIGSEGV were not received in handler
    assert((signalsReceived & (1 << SIGFPE)) == 0);
    assert((signalsReceived & (1 << SIGSEGV)) == 0);

    // Remove SIGSEGV from the mask and unblock with mask (unblock SIGFPE)
    sigdelset(&sigMask, SIGSEGV);
    ret = pthread_sigmask(SIG_UNBLOCK, &sigMask, NULL);
    assert(0 == ret);

    // Verify SIGFPE was received in handler
    assert((signalsReceived & (1 << SIGFPE)));

    // Unblock SIGSEGV
    sigemptyset(&sigMask);
    sigaddset(&sigMask, SIGSEGV);
    ret = pthread_sigmask(SIG_UNBLOCK, &sigMask, NULL);
    assert(0 == ret);

    // Verify SIGSEGV was received in handler
    assert((signalsReceived & (1 << SIGSEGV)));

    // Block SIGTRAP (after that raise it)
    sigemptyset(&sigMask);
    sigaddset(&sigMask, SIGTRAP);
    ret = pthread_sigmask(SIG_BLOCK, &sigMask, NULL);
    assert(0 == ret);

    raise(SIGTRAP);

    // Verify SIGTRAP wasn't not received in handler
    assert((signalsReceived & (1 << SIGTRAP)) == 0);

    // Retrieve current mask
    ret = pthread_sigmask(SIG_SETMASK, NULL, &sigMask);
    assert(0 == ret);

    // Unblock SIGTRAP by removing SIGTRAP from mask and then using SIG_SETMASK.
    // This checks another interesting scenario
    sigdelset(&sigMask, SIGTRAP);
    ret = pthread_sigmask(SIG_SETMASK, &sigMask, NULL);
    assert(0 == ret);

    // Verify SIGTRAP was received in handler
    assert((signalsReceived & (1 << SIGTRAP)));

    // Verify All signals received by handler
    if (signalsReceived == ALL_SYNC_SIGNALS)
    {
        printf("All signals were received in first phase\n");
    }
    else
    {
        printf("Not all signals were received in first phase\n");
        return 1;
    }

    //
    // Phase 2, raise a few signals more than once  before unblocking (Another case we want to check)
    //

    // Clear signalsReceived first
    signalsReceived = 0;

    sigemptyset(&sigMask);
    sigaddset(&sigMask, SIGILL);
    sigaddset(&sigMask, SIGBUS);
    ret = pthread_sigmask(SIG_BLOCK, &sigMask, NULL);
    assert(0 == ret);

    // SIGILL 3 times, SIGBUS 2 times
    raise(SIGILL);
    raise(SIGSEGV);
    raise(SIGBUS);
    raise(SIGTRAP);
    raise(SIGILL);
    raise(SIGBUS);
    raise(SIGILL);

    ret = pthread_sigmask(SIG_UNBLOCK, &sigMask, NULL);
    assert(0 == ret);

    raise(SIGFPE);

    // Check again
    if (signalsReceived == ALL_SYNC_SIGNALS)
    {
        printf("All signals were received in second phase\n");
        return 0;
    }
    else
    {
        printf("Not all signals were received in second phase\n");
        return 1;
    }
}

int CheckAysncExceptions()
{
    struct sigaction act;
    bzero(&act, sizeof(act));
    act.sa_flags     = SA_SIGINFO;
    act.sa_sigaction = Handler;

    sigaction(SIGFPE, &act, NULL);
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGTRAP, &act, NULL);

    raise(SIGFPE);
    raise(SIGSEGV);
    raise(SIGILL);
    raise(SIGBUS);
    raise(SIGTRAP);

    if (signalsReceived == ALL_SYNC_SIGNALS)
    { //Verifying all signals were handled by the handler
        printf("All signals were received\n");
        return 0;
    }
    else
    {
        printf("Not all signals were received\n");
        return 1;
    }
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        if (atoi(argv[1]) == 1)
        {
            return CheckBlockedAysncExceptions();
        }
        else
        {
            return 1;
        }
    }

    return CheckAysncExceptions();
}

void Handler(int signum, siginfo_t* siginfo, void* _uctxt)
{
    signalsReceived |= (1 << signum);
    printf("received signal %d\n", signum);
}

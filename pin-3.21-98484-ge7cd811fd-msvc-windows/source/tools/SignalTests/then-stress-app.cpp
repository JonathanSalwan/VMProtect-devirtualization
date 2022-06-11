/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This is a test program that stresses Pin's ability to deliver signals while
 * executing in a tool's "then" analysis routine.  This application is meant to
 * run with the "then-stress-tool.cpp" tool.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

static const unsigned SIGCOUNT = 100;

typedef void (*FN)();
volatile unsigned SigCount       = 0;
volatile unsigned long LoopCount = 0;

static void Handle(int);
extern "C" void DoThenInstrumentation();

int main()
{
    struct sigaction sigact;
    sigact.sa_handler = Handle;
    sigact.sa_flags   = 0;
    sigemptyset(&sigact.sa_mask);
    if (sigaction(SIGALRM, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable to set up handler\n");
        return 1;
    }

    struct itimerval itval;
    itval.it_interval.tv_sec  = 0;
    itval.it_interval.tv_usec = 100000;
    itval.it_value.tv_sec     = 0;
    itval.it_value.tv_usec    = 100000;
    if (setitimer(ITIMER_REAL, &itval, 0) == -1)
    {
        fprintf(stderr, "Unable to set up timer\n");
        return 1;
    }

    volatile FN doThenInstrumentation = DoThenInstrumentation;
    while (SigCount < SIGCOUNT)
    {
        doThenInstrumentation();
        LoopCount++;
    }

    itval.it_value.tv_sec  = 0;
    itval.it_value.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &itval, 0) == -1)
    {
        fprintf(stderr, "Unable to disable timer\n");
        return 1;
    }

    return 0;
}

static void Handle(int sig)
{
    SigCount++;
    printf("SigCount=%u, LoopCount=%lu\n", SigCount, LoopCount);
    fflush(stdout);
}

void DoThenInstrumentation() { /* the Pin tool inserts if-then instrumentation here */ }

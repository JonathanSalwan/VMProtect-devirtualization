/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that Pin properly saves and restores the applicaton's
 * FP state when emulating a delivered signal.  The application's main thread
 * simply does some FP calculation in a loop while ALRM signals are handled.
 * The signal handler also does some FP computation, which will change the
 * FP state.  If Pin doesn't properly save/restore the FP state, the handler
 * will mess up the main thread's calculations.
 *
 *********************************************************************************
 *
 * I have observed that this test occasionally fails on vs-lin64-4 (running SUSE10
 * on Intel64).  The cause of the problem is probably the same as the bug
 * described at the top of "xmmcheck.c".
 */

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#define COUNT 100

volatile unsigned SigCount = 0;

static double fpcheck();
static void handle1(int);
static void handle2(int, siginfo_t*, void*);

int main()
{
    struct sigaction sigact;
    struct itimerval itval;
    double fact;
    char buf[100];
    char* point;

    printf("Before sigaction call\n");
    sigact.sa_handler = handle1;
    sigact.sa_flags   = 0;
    sigemptyset(&sigact.sa_mask);
    if (sigaction(SIGALRM, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable to set up handler\n");
        return 1;
    }

    printf("Before setitimer\n");
    itval.it_interval.tv_sec  = 0;
    itval.it_interval.tv_usec = 10000;
    itval.it_value.tv_sec     = 0;
    itval.it_value.tv_usec    = 10000;
    if (setitimer(ITIMER_REAL, &itval, 0) == -1)
    {
        fprintf(stderr, "Unable to set up timer\n");
        return 1;
    }

    printf("Before fpcheck\n");
    fact = fpcheck();
    printf("After fpcheck\n");

    itval.it_value.tv_sec  = 0;
    itval.it_value.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &itval, 0) == -1)
    {
        fprintf(stderr, "Unable to disable timer\n");
        return 1;
    }

    sprintf(buf, "%.0f", fact);
    point = strchr(buf, '.');
    if (point) *point = '\0';
    if (strcmp(buf, "1307674368000") != 0)
    {
        fprintf(stderr, "15! should be 1307674368000, but computed as '%s'\n", buf);
        return 1;
    }
    printf("15! = %s\n", buf);
    return 0;
}

static double fpcheck()
{
    double x[15];
    double fact, calc;
    unsigned i;

    for (i = 1; i <= 15; i++)
        x[i - 1] = (double)i;

    fact = 1.0;
    for (i = 0; i < 15; i++)
        fact = fact * x[15 - (i + 1)];

    while (SigCount < COUNT)
    {
        calc = 1.0;
        for (i = 0; i < 15; i++)
            calc = calc * x[15 - (i + 1)];
        if ((calc - fact > 0.5) || (calc - fact < -0.5))
        {
            fprintf(stderr, "Unexpected FP change (fact=%f, calc=%f)\n", fact, calc);
            return 1;
        }
    }

    return fact;
}

static void handle1(int sig)
{
    SigCount++;
    double x[100];
    unsigned i;

    fprintf(stderr, "handle1: %d\n", SigCount);

    for (i = 0; i < 100; i++)
        x[i] = (double)(i + 1);
    for (i = 2; i < 100; i++)
        x[i] = x[i] / x[i - 1] * x[i - 2] + x[i];

    /*
     * After a while, switch to using a "siginfo" handler.  This exercises different
     * signal emulation paths within Pin.
     */
    if (SigCount > COUNT / 2)
    {
        struct sigaction sigact;

        sigact.sa_sigaction = handle2;
        sigact.sa_flags     = SA_SIGINFO;
        sigemptyset(&sigact.sa_mask);
        if (sigaction(SIGALRM, &sigact, 0) == -1)
        {
            fprintf(stderr, "Unable to reset handler\n");
            exit(1);
        }
    }
}

static void handle2(int sig, siginfo_t* si, void* v)
{
    double x[100];
    unsigned i;

    SigCount++;

    fprintf(stderr, "handle1: %d\n", SigCount);

    for (i = 0; i < 100; i++)
        x[i] = (double)(i + 1);
    for (i = 2; i < 100; i++)
        x[i] = x[i] / x[i - 1] * x[i - 2] + x[i];
}

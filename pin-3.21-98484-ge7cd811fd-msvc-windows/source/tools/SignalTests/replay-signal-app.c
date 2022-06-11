/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application is used to test Pin's PIN_ReplayUnixSignal() API.
 * It must be run with the Pin tool "replay-signal-tool.cpp".
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

typedef void (*PF)();
volatile PF pfReplaySignal1;

void Handle(int);
void ReplaySignal1();
void ReplaySignal2();

static int First = 1;

int main()
{
    struct sigaction sigact;

    sigact.sa_handler = Handle;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    if (sigaction(SIGUSR1, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable to handle signal\n");
        return 1;
    }

    raise(SIGUSR1);

    pfReplaySignal1 = ReplaySignal1;
    pfReplaySignal1();

    fprintf(stderr, "Should not get here (1)\n");
    return 1;
}

void Handle(int sig)
{
    if (sig != SIGUSR1)
    {
        fprintf(stderr, "Expected signal %d (SIGUSR1), but got %d\n", SIGUSR1, sig);
        exit(1);
    }

    printf("Got signal SIGUSR1\n");
    fflush(stdout);

    if (First)
    {
        First = 0;
        return;
    }

    pfReplaySignal1 = ReplaySignal2;
    pfReplaySignal1();

    fprintf(stderr, "Should not get here (2)\n");
    exit(1);
}

void ReplaySignal1()
{
    /*
     * Pin puts an instrumentation point here to replay a handled SIGUSR1 signal.
     */
}

void ReplaySignal2()
{
    /*
     * Pin puts an instrumentation point here to replay a fatal SIGUSR1 signal.
     */
}

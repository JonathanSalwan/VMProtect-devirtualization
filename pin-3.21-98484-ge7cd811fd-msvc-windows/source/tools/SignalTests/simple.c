/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static void Handle(int);
static void HandlerIsEstablished();

typedef void (*PF)();
volatile PF pfEstablished;

int main()
{
    struct sigaction act;

    act.sa_handler = Handle;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGUSR1, &act, 0) != 0)
    {
        fprintf(stderr, "Unable to set up USR1 handler\n");
        return 1;
    }

    pfEstablished = HandlerIsEstablished;
    pfEstablished();

    raise(SIGUSR1);
    raise(SIGUSR1);

    fprintf(stderr, "Should exit from handler\n");
    return 1;
}

static void Handle(int sig)
{
    static int Count = 0;

    printf("In handler\n");
    if (++Count == 2) exit(0);
}

static void HandlerIsEstablished()
{
    /*
     * Pin can add an instrumentation point here to be notified
     * when the signal handler is established.
     */
}

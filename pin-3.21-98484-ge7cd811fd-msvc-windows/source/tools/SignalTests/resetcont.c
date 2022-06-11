/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <signal.h>
#include <stdio.h>

void handle(int);

int main()
{
    struct sigaction sigact;

    sigact.sa_handler = handle;
    sigact.sa_flags   = SA_RESETHAND;
    sigemptyset(&sigact.sa_mask);
    if (sigaction(SIGCONT, &sigact, 0) == -1)
    {
        printf("Unable to handle signal\n");
        return 1;
    }

    raise(SIGCONT);
    raise(SIGCONT);

    printf("Exiting after second SIGCONT\n");
    return 0;
}

void handle(int sig)
{
    if (sig == SIGCONT)
        printf("Got signal CONT\n");
    else
        printf("Got signal %d\n", sig);
    fflush(stdout);
}

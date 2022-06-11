/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

static void HandleUSR1(int);
static void HandleUSR2(int);

int main()
{
    struct sigaction sigact;

    sigact.sa_handler = HandleUSR1;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    if (sigaction(SIGUSR1, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable handle signal USR1\n");
        return 1;
    }
    sigact.sa_handler = HandleUSR2;
    if (sigaction(SIGUSR2, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable handle signal USR2\n");
        return 1;
    }

    raise(SIGUSR1);
    return 0;
}

static void HandleUSR1(int sig)
{
    raise(SIGUSR2);
    raise(SIGUSR2);
}

static void HandleUSR2(int sig)
{
    static int count = 0;

    if (++count == 2) exit(0);
}

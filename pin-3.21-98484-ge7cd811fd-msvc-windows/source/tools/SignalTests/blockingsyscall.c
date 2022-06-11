/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that Pin correctly delivers a signal during a blocking system call.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

unsigned SigCount = 0;

static void Handle(int);

int main()
{
    struct sigaction sigact;
    ssize_t sz;
    char buf;

    sigact.sa_handler = Handle;
    sigact.sa_flags   = 0;
    sigemptyset(&sigact.sa_mask);
    if (sigaction(SIGALRM, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable to set up handler\n");
        return 1;
    }

    for (;;)
    {
        alarm(3);
        pause();
    }

    printf("Shouldn't get here!!\n");
    return 1;
}

static void Handle(int sig)
{
    printf("Got SIGALRM\n");
    SigCount++;
    if (SigCount > 1) exit(0);
}

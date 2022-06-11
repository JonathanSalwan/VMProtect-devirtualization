/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sys/types.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

extern void DoRelease(volatile bool* doRelease)
{
    // do nothing
}

static void WaitForAttach()
{
    const unsigned int timeout  = 300;
    unsigned int releaseCounter = 0;
    volatile bool released      = false;
    while (!released)
    {
        if (releaseCounter >= timeout)
        {
            printf("APP ERROR: Timeout reached and the tool did not release the application.\n");
            exit(1);
        }
        ++releaseCounter;
        DoRelease(&released);
        sleep(1);
    }
}

void sig_handler_1(int unused) { printf("Caught signal SIGSEGV \n"); }

int main()
{
    signal(SIGSEGV, sig_handler_1);
    printf("Test process: %d\n", getpid());

    WaitForAttach();
    pthread_kill(pthread_self(), SIGSEGV);

    return 0;
}

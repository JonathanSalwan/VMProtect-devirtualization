/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that Pin can emulate a weird signal.
 * The application sends itself an asynchronous signal, but
 * uses a signal number that is normally a synchronous signal.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static void Handle(int, siginfo_t*, void*);

int main()
{
    struct sigaction act;
    act.sa_sigaction = Handle;
    act.sa_flags     = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGFPE, &act, 0) != 0)
    {
        printf("Unable to set up FPE handler\n");
        return 1;
    }

    kill(getpid(), SIGFPE);
    /* should not return */

    printf("Should not return from signal handler\n");
    return 1;
}

static void Handle(int sig, siginfo_t* info, void* ctxt)
{
    if (sig != SIGFPE)
    {
        printf("Got unexpected signal %d\n", sig);
        exit(1);
    }
    if (info->si_code != 0)
    {
        printf("Expected si_code to be zero, but got %d\n", (int)info->si_code);
        exit(1);
    }
    exit(0);
}

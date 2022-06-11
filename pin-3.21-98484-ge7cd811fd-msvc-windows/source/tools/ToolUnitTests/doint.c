/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static void SetupSigHandler();
static void Handle(int);
extern void DoInt();

int main()
{
    SetupSigHandler();
    DoInt();
    return 0;
}

static void SetupSigHandler()
{
    struct sigaction act;

    act.sa_handler = Handle;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGSEGV, &act, 0);
    sigaction(SIGILL, &act, 0);
}

static void Handle(int sig)
{
    printf("Got signal %d\n", sig);
    exit(0);
}

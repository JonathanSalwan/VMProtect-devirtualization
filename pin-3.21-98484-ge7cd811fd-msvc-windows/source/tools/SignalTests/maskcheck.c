/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test prints the application's blocked signal mask at various points.
 * By comparing a native run with a run under Pin, we can verify that Pin emulates
 * the signal mask correctly.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/ucontext.h>

#if defined(TARGET_MAC)
#define MAXMASK 32
#else
#define MAXMASK 64
#endif

static void Handle(int);
static void PrintCurrentMask(const char*);
static void PrintMask(const char*, sigset_t*);

int main()
{
    struct sigaction act;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGSTOP);
    sigaddset(&mask, SIGFPE);
    if (sigprocmask(SIG_SETMASK, &mask, 0) != 0)
    {
        fprintf(stderr, "unable to set blocked signal mask\n");
        return 1;
    }

    PrintCurrentMask("Initial: ");

    act.sa_handler = Handle;
    act.sa_flags   = SA_NODEFER;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGSEGV);
    sigaddset(&act.sa_mask, SIGHUP);
    sigaddset(&act.sa_mask, SIGKILL);
    if (sigaction(SIGUSR1, &act, 0) != 0)
    {
        fprintf(stderr, "Unable to set USR1 handler\n");
        return 1;
    }

    act.sa_flags = 0;
    if (sigaction(SIGUSR2, &act, 0) != 0)
    {
        fprintf(stderr, "Unable to set USR2 handler\n");
        return 1;
    }

    raise(SIGUSR1);
    PrintCurrentMask("After USR1: ");

    raise(SIGUSR2);
    PrintCurrentMask("After USR2: ");

    sigemptyset(&act.sa_mask);
    if (sigaction(SIGUSR1, 0, &act) != 0)
    {
        fprintf(stderr, "Unable to get USR1 hander\n");
        return 1;
    }
    PrintMask("USR1 Blocks: ", &act.sa_mask);

    sigemptyset(&act.sa_mask);
    if (sigaction(SIGUSR2, 0, &act) != 0)
    {
        fprintf(stderr, "Unable to get USR2 hander\n");
        return 1;
    }
    PrintMask("USR2 Blocks: ", &act.sa_mask);

    return 0;
}

static void Handle(int sig)
{
    if (sig == SIGUSR1)
        PrintCurrentMask("USR1 handler: ");
    else if (sig == SIGUSR2)
        PrintCurrentMask("USR2 handler: ");
    else
    {
        fprintf(stderr, "Unexpected signal %d\n", sig);
        exit(1);
    }
}

static void PrintCurrentMask(const char* prefix)
{
    sigset_t mask;

    if (sigprocmask(SIG_SETMASK, 0, &mask) == 0)
        PrintMask(prefix, &mask);
    else
        printf("%s[ERROR]\n", prefix);
}

static void PrintMask(const char* prefix, sigset_t* mask)
{
    int sig;
    int first;

    first = 1;
    printf("%s[", prefix);
    for (sig = 1; sig < MAXMASK; sig++)
    {
        if (sigismember(mask, sig))
        {
            if (!first) printf(" ");
            first = 0;
            printf("%d", sig);
        }
    }
    printf("]\n");
}

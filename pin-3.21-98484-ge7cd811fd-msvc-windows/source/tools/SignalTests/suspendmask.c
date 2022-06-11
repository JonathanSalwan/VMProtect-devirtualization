/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that the signal mask is set correctly in
 * a handler when the signal interrupts a sigsuspend() call.
 * The test should have the same output when run natively or
 * under Pin.
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <sys/ucontext.h>

static void Handle(int sig, siginfo_t* info, void* vctxt);
static void CheckSigs(sigset_t* ss, const char* mesg);

int main()
{
    struct sigaction act;
    sigset_t ss;

    act.sa_sigaction = Handle;
    act.sa_flags     = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGUSR1, &act, 0) != 0)
    {
        fprintf(stderr, "unable to set SIGUSR1 handler\n");
        return 1;
    }
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGUSR2, &act, 0) != 0)
    {
        fprintf(stderr, "unable to set SIGUSR2 handler\n");
        return 1;
    }
    sigfillset(&ss);
    sigprocmask(SIG_SETMASK, &ss, 0);
    kill(getpid(), SIGUSR1);
    kill(getpid(), SIGUSR2);

    sigdelset(&ss, SIGUSR1);
    sigdelset(&ss, SIGUSR2);
    sigdelset(&ss, SIGSEGV);
    sigsuspend(&ss);

    sigprocmask(SIG_SETMASK, 0, &ss);
    CheckSigs(&ss, "After signal");
    return 0;
}

static void Handle(int sig, siginfo_t* info, void* vctxt)
{
    ucontext_t* ctxt = vctxt;
    sigset_t ss;

    printf("Received %s\n", (sig == SIGUSR1) ? "USR1" : "USR2");
    sigprocmask(SIG_SETMASK, 0, &ss);
    CheckSigs(&ss, "In handler");
    CheckSigs(&ctxt->uc_sigmask, "Previous mask");
}

static void CheckSigs(sigset_t* ss, const char* mesg)
{
    printf("%s:\n", mesg);
    if (sigismember(ss, SIGUSR1))
        printf(" USR1 is blocked\n");
    else
        printf(" USR1 is not blocked\n");
    if (sigismember(ss, SIGUSR2))
        printf(" USR2 is blocked\n");
    else
        printf(" USR2 is not blocked\n");
    if (sigismember(ss, SIGSEGV))
        printf(" SEGV is blocked\n");
    else
        printf(" SEGV is not blocked\n");
}

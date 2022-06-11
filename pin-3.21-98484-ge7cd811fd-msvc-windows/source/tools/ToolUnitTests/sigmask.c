/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// Check that signal masks are handled properly for signals

#include <stdio.h>
#include <signal.h>

void CheckSigs()
{
    sigset_t sigset;
    sigprocmask(SIG_BLOCK, 0, &sigset);

    if (sigismember(&sigset, SIGUSR1))
    {
        fprintf(stderr, "usr1 blocked\n");
    }
    if (sigismember(&sigset, SIGUSR2))
    {
        fprintf(stderr, "usr2 blocked\n");
    }
    if (sigismember(&sigset, SIGTERM))
    {
        fprintf(stderr, "term blocked\n");
    }
    if (sigismember(&sigset, SIGWINCH))
    {
        fprintf(stderr, "winch blocked\n");
    }
    fprintf(stderr, "\n");
}

void Handler(int n)
{
    fprintf(stderr, "In handler\n");
    fprintf(stderr, "--------------\n");
    CheckSigs();
}

int main()
{
    sigset_t sigset;

    struct sigaction sa;
    sa.sa_handler = Handler;
    sa.sa_flags   = 0;

    // Block sigterm in the handler
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);

    // Block USR1 here
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigset, 0);

    sigaction(SIGWINCH, &sa, 0);

    fprintf(stderr, "Before signal1\n");
    fprintf(stderr, "--------------\n");
    CheckSigs();

    // the kernel blocks SIGWINCH in the handler
    kill(0, SIGWINCH);

    fprintf(stderr, "After signal1\n");
    fprintf(stderr, "--------------\n");
    CheckSigs();

    // No defer should not block SIGWINCH in handler
    sa.sa_flags |= SA_NODEFER;
    sigaction(SIGWINCH, &sa, 0);
    kill(0, SIGWINCH);

    fprintf(stderr, "After signal2\n");
    fprintf(stderr, "--------------\n");
    CheckSigs();

    return 0;
}

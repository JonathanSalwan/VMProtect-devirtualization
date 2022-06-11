/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that a tool can intercept a signal that the application
 * blocks.  The application blocks all signals and the tool verifies that it
 * can still receive its intercepted signal.
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main()
{
    sigset_t mask;

    sigfillset(&mask);
    sigdelset(&mask, SIGALRM);
    sigprocmask(SIG_SETMASK, &mask, 0);
    printf("Signals are blocked\n");
    fflush(stdout);

    alarm(60); /* kills the process if the test hangs for some reason*/

    for (;;)
        ;
    return 0;
}

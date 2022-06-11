/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <unistd.h>
#include <signal.h>

int main()
{
    struct sigaction act;

    // Make sure SIGINT isn't ignored.  On some systems, SIGINT is initially
    // ignored when this test is run from scons / make.
    //
    act.sa_handler = SIG_DFL;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, 0);

    kill(getpid(), SIGINT);
    return 0;
}

/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <signal.h>

static void Handle(int);

int main()
{
    struct sigaction act;
    act.sa_handler = Handle;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGUSR1, &act, 0) != 0)
    {
        std::cerr << "Unable to establish handler\n";
        return 1;
    }

    raise(SIGUSR1);
    return 0;
}

static void Handle(int sig) {}

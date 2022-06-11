/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that Pin can emulate signal #64, which is the last signal on Linux.
 */

#include <iostream>
#include <signal.h>
#include <unistd.h>

#ifdef TARGET_BSD
#ifndef SIGRTMAX
#define SIGRTMAX 128
#endif
#endif

#ifdef TARGET_MAC
#ifndef SIGRTMAX
#define SIGRTMAX 32
#endif
#endif

static void Handle(int sig);

int main()
{
    int maxSig = SIGRTMAX;
    std::cout << "SIGRTMAX=" << maxSig << "\n";

    struct sigaction act;
    act.sa_handler = Handle;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);
    int ret = sigaction(64, &act, 0);
    std::cout << "sigaction(64) returns: " << ret << "\n";

    sigset_t set;
    ret = sigaddset(&set, 64);
    std::cout << "sigaddset(64) returns: " << ret << "\n";

    ret = kill(getpid(), 64);
    std::cout << "kill(64) returns: " << ret << "\n";

    return 0;
}

static void Handle(int sig) { std::cout << "Got signal " << sig << "\n"; }

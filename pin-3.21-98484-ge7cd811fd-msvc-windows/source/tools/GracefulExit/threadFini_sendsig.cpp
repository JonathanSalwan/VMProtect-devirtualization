/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This program receives a pid as its argument and sends a SIGTERM to it.
 */

#include <cassert>
#include <cstdlib>
#include <signal.h>
#include <sys/types.h>

// argv[1] is expected to be the pid to kill.
int main(int argc, char** argv)
{
    assert(argc == 2);
    int pid = atoi(argv[1]);
    assert(pid > 1);
    kill(pid, SIGTERM);
    return 0;
}

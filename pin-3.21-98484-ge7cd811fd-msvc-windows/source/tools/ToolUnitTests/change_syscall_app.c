/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application must be run with Pin tool "change_syscall.cpp".  That tool
 * changes the system calls that this application executes, and the test only
 * passes if that happens.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>

int main()
{
    struct sigaction sa;
    int ret;

    /*
     * The tool should change "open" to "getpid".  This tests that a non-emulated
     * system call can be changed to another non-emulated system call.
     */
    ret = open("does-not-exist1", 0);
    if (ret != getpid())
    {
        printf("open() system call not changed, ret = %d\n", ret);
        return 1;
    }

    /*
     * The tool should change "sigaction" to "getpid".  This tests that an
     * emulated system call can be changed to a non-emulated system call.
     */
    sa.sa_handler = 0;
    sa.sa_flags   = 0;
    sigemptyset(&sa.sa_mask);
    ret = sigaction(SIGUSR1, &sa, 0);
    if (ret != getpid())
    {
        printf("sigaction() system call not changed, ret = %d\n", ret);
        return 1;
    }

    /*
     * The tool should change "open" to "exit(0)".  This tests that a non-emulated
     * system call can be changed into an emulated one.
     */
    open("does-not-exist2", 0);

    printf("open was not changed to exit\n");
    return 1;
}

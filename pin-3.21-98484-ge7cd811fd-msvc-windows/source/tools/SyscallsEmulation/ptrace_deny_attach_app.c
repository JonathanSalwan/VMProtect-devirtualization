/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * Testing ptrace with PT_DENY_ATTACH is ignored by Pin (flag available only in macOS).
 * Application may use this flag to deny being attached using ptrace system call.
 * This will not be an issue when Pin is launching such an application, since it uses ptrace before running any application code
 * and let it run "natively" after that (without ptrace-ing it)
 * However PT_DENY_ATTACH will cause a segmentation fault in the following cases:
 * 1. Pin decide to use ptrace again in one of his flows
 * 2. A user want to attach with a debugger to the process for debugging
 * 3. Pin attach to an application which has already used this flag
 *
 * This test only checks that Pin knows to handle cases 1 and 2.
 * It doesn't check case 3, since Pin currently doesn't have a way to handle it (see mantis #3981 for more information)
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

int main()
{
    int parentToChildPipe[2];
    int ChildToParentPipe[2];
    pid_t child;
    int res;

    printf("Application started\n");

    if (pipe(parentToChildPipe) < 0)
    {
        return 1;
    }
    if (pipe(ChildToParentPipe) < 0)
    {
        return 1;
    }

    child = fork();
    if (child == 0)
    { // Child process
        int ret = -1;
        close(parentToChildPipe[1]);
        close(ChildToParentPipe[0]);

        // Deny attach in child
        printf("Deny PTRACE requests from Child\n");
        errno = 0;
        ret   = ptrace(PT_DENY_ATTACH, 0, 0, 0);
        // Pin ignore PT_DENY_ATTACH but still need to emulate a correct return value
        assert(errno == 0);
        assert(ret == 0);

        // Notify Parent, child is ready to be traced
        close(ChildToParentPipe[1]);

        // Wait for parent process to notify it has attach to this process (child) before continuing
        do
        {
            char dummy;
            res = read(parentToChildPipe[0], &dummy, sizeof(dummy));
        }
        while (res < 0 && errno == EINTR);
        assert(res == 0);

        return 0;
    }
    else
    { // Parent process

        long ret;
        pid_t wpid;
        close(parentToChildPipe[0]);
        close(ChildToParentPipe[1]);

        // Wait for child to notify it is ready to be traced
        do
        {
            char dummy;
            res = read(ChildToParentPipe[0], &dummy, sizeof(dummy));
        }
        while (res < 0 && errno == EINTR);
        assert(res == 0);

        // Attach to child
        ret = ptrace(PT_ATTACH, child, 0, 0);
        if (ret < 0)
        {
            exit(1);
        }
        // Wait for child (tracee) to stop
        wpid = waitpid(child, NULL, WUNTRACED);
        if (wpid != child)
        {
            exit(1);
        }

        // Let child (tracee) continue
        ret = ptrace(PT_CONTINUE, child, (caddr_t)1, 0);
        if (ret < 0)
        {
            exit(1);
        }

        // Notify child, parent has attached to it and it can continue
        close(parentToChildPipe[1]);
    }

    printf("Application Finished, bye...\n");
    return 0;
}

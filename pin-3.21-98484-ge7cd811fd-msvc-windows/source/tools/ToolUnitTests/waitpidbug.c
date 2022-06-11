/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#define MAX_COMMAND_LINE_SIZE 15 // the size for the array of arguments to execv (this value is arbitrary)

/*
 * This program tests for a bug with waitpid. We run pin in a child and
 * wait for it. If pin uses ptrace attach to inject itself, the waitpid
 * fails with ECHILD. This appears to be a bug in waitpid that is fixed in
 * 2.6 kernels. Pin works around the problem by not using attach to inject
 * itself on 2.4 systems. This test checks the above mentioned workaround.
 * The criteria for success are that waitpid does not fail, i.e. returns
 * with the pid of the child, and that the child ran Pin succesfully.
 */

/*
 * Expected argv arguments:
 * [1] pin executable
 * [2] Pin flags (e.g. -slow_asserts)
 *     >> zero or more flags possible
 * [3] copy application
 * [4] copy source
 * [5] copy target
 */
main(int argc, char* argv[])
{
    if (argc > MAX_COMMAND_LINE_SIZE)
    {
        fprintf(stderr, "Too many arguments\n");
        fflush(stderr);
        exit(1);
    }
    pid_t pid = fork();
    if (pid)
    {
        while (1)
        {
            int status;
            pid_t cpid = waitpid(0, &status, WNOHANG);
            if (cpid > 0)
            {
                fprintf(stderr, "Child pid: %d\n", cpid);
                if (WIFEXITED(status))
                {
                    int res = WEXITSTATUS(status);
                    fprintf(stderr, "Child exited with value %d\n", res);
                    exit(res);
                }
                else
                {
                    exit(1);
                }
            }
            if (cpid < 0)
            {
                perror("wait:");
                exit(1);
            }

            sched_yield();
        }
    }
    else
    {
        char* args[MAX_COMMAND_LINE_SIZE] = {NULL}; // arguments for execv command
        int args_count                    = 0;
        int argv_count                    = 1; // to start from argv[1]...
        while (argv_count < argc)
        {
            args[args_count++] = argv[argv_count++]; // all arguments including Pin flags will be passed to execv
        }
        args[args_count++] = NULL;          // end
        execv(argv[1], (char* const*)args); // never returns
        fprintf(stderr, "execv failed with errno: %d\n", errno);
    }
}

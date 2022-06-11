/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

char* myExe = NULL;

void executeChildProcess(int pip)
{
    int my_ppid = getppid();
    size_t res  = (int)write(pip, &my_ppid, sizeof(my_ppid));
    assert(res == sizeof(my_ppid));
    close(pip);
}

void executeParentProcess(pid_t child, int pip)
{
    int reported_pid = -1;
    int status       = -1;
    int res          = (int)read(pip, &reported_pid, sizeof(reported_pid));
    assert(res == sizeof(reported_pid));
    close(pip);
    while (0 > (res = waitpid(child, &status, 0)) && errno == EINTR)
        ;
    assert(res == child);
    assert(status == 0);
    printf("my pid %d, ppid reported from child: %d\n", (int)getpid(), reported_pid);
    assert((int)getpid() == reported_pid);
}

void testFork(int shouldExec)
{
    int pip[2];
    int res = pipe(pip);
    assert(res == 0);
    pid_t child = fork();
    assert(child >= 0);
    if (0 == child)
    {
        if (shouldExec)
        {
            char pip_str[16];
            char* argv[3] = {myExe, pip_str, NULL};
            snprintf(pip_str, sizeof(pip_str), "%d", pip[1]);
            close(pip[0]);
            execv(myExe, argv);
            assert(0);
        }
        else
        {
            close(pip[0]);
            executeChildProcess(pip[1]);
            _exit(0);
        }
    }
    else
    {
        close(pip[1]);
        executeParentProcess(child, pip[0]);
    }
}

int main(int argc, char* argv[])
{
    myExe = argv[0];
    if (argc == 2)
    {
        char* endptr;
        errno         = 0;
        const int pip = (int)strtol(argv[1], &endptr, 10);
        assert(*endptr == 0 && pip >= 0 && 0 == errno);
        executeChildProcess(pip);
    }
    else
    {
        testFork(0);
        testFork(1);
    }
    return 0;
}

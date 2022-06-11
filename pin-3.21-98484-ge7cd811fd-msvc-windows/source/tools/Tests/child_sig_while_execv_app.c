/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

/*

This test creates the scenario in which prcocess that runs under Pin that fail in execv syscall.
The process in this case calls waitpid to wait for its injector to exit, in this point the process
receives a siganl and exit from the waitpid syscall with errno==EINTR.

To generate this scenario the application creates 10 child processes that sends many SIGCHLD to parent process.
The parent process use execvp to run simple application located in ./obj-intel64 .
The execvp tries to run the simple application by activating execv on each directory in system PATH.
This is causing many failures in execv syscall activation.

The scenario of many SIGCHLD signals along with many failures in execv bring us to the requested case.

*/

static void hdl(int sig, siginfo_t* siginfo, void* context) {}

int main(int argc, char** argv)
{
    char* args[]  = {"child_sig_while_execv", "0", NULL};
    int iteration = 0;
    int child_pid = 0;
    int parent_id = getpid();
    int i;

    // Set long path to make sure execvp will fail on many execv tries
    setenv(
        "PATH",
        "/usr/intel/bin:/sbin:/bin:/usr/sbin:/usr/X386/bin:/usr/local:/usr/local/bin:/usr/ucb:/usr/afsws/bin:/usr/afsws/etc:/usr/"
        "ccs/bin:/opt/kde3/bin:/usr/kerberos/bin:/usr/local/bin:/bin:/usr/bin:/usr/X11R6/bin:/bin:/tmp:/tmp_proj:.:./obj-intel64",
        1);

    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    // Use the sa_sigaction field because the handles has two additional parameters
    act.sa_sigaction = &hdl;
    // The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler.
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGCHLD, &act, NULL) < 0)
    {
        perror("sigaction");
        return 1;
    }

    int fd[2];
    char buf[2];
    if (pipe(fd) == -1)
    {
        fprintf(stderr, "Unable to open pipe ");
        return 1;
    }

    // create 10 child processes
    for (i = 0; i < 10; i++)
        if (parent_id == getpid()) fork();

    if (parent_id == getpid()) // if parent
    {
        close(fd[0]); // Close unused read end
        close(fd[1]); // send EOF to signal exec start
        execvp("child_sig_while_execv_simple_app.exe", args);

        // In case execvp fail
        perror("EXECV ERROR: exec failed");
        return 1;
    }

    close(fd[1]);         // Close unused write end
    read(fd[0], &buf, 1); // wait until parent ready for exec
    close(fd[0]);

    // send SIGCHLD to parent, one of them will hit when parent is waiting on waitpid
    // and will cause it to exit the waitpid with errno==EINTR.
    for (i = 0; i < 15; i++)
        kill(parent_id, SIGCHLD);
    return 0;
}

/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application is used by the tests: "child_process_injection1.test" and "child_process_injection1_mac.test"
 * The argument passed should be an app with setuid privileges. In Linux chfn is used when available, whilst in macOS* a
 * utility is built and its privileges are altered.
 * For this test we run pin with -follow-execv, so pin will try to inject and should be unsuccessful due to privilege, but we
 * still expect the child to run natively.
 */

#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    pid_t pid = fork();
    if (pid == 0)
    {
        int err = execv(argv[1], argv + 1);
        fprintf(stdout, "%s\n", "Child report: Execve failed ");
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        fprintf(stdout, "%s%d\n", "status", status);
        if (status != 0)
            fprintf(stdout, "%s%d\n", "Parent report: Child process failed. Status of the child process is ",
                    WEXITSTATUS(status));
        else
            fprintf(stdout, "%s\n", "Parent report: Child process exited successfully");
    }
    return 0;
}

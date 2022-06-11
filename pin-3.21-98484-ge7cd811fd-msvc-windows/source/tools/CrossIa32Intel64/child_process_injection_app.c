/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application is used by the test: "child_process_injection.test"
 * This application is used to check the correctness of a successfull Pin injection to a child process.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <executable to run>\n", argv[0]);
        return 1;
    }
    pid_t pid = fork();
    if (pid == 0)
    {
        char* childArgvArray[2];
        childArgvArray[0] = argv[1];
        childArgvArray[1] = NULL;
        execv(childArgvArray[0], childArgvArray);
        fprintf(stdout, "%s\n", "Child report: Execve failed ");
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        if (status != 0)
            fprintf(stdout, "%s%d\n", "Parent report: Child process failed. Status of the child process is ",
                    WEXITSTATUS(status));
        else
            fprintf(stdout, "%s\n", "Parent report: Child process exited successfully");
    }
    return 0;
}

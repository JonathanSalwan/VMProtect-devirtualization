/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/* Application to test that LD_BIND_NOW is not set in the application */
/* ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

void check_ld_bind()
{
    const char* bind = getenv("LD_BIND_NOW");
    if (bind)
    {
        fprintf(stderr, "Found LD_BIND_NOW, exiting...\n");
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    int status = 0;
    pid_t pid;

    check_ld_bind();

    if ((pid = fork()) == 0)
    {
        // In the child, here again check for LD_BIND_NOW
        check_ld_bind();
        if (argc > 1 && strcmp(argv[1], "-child") == 0)
        {
            // execute the same application but without params
            char* childArgvArray[2];
            childArgvArray[0] = argv[0];
            childArgvArray[1] = NULL;
            fprintf(stderr, "Going to execv...\n");
            execv(childArgvArray[0], childArgvArray);
            fprintf(stderr, "execv failed, exiting...\n");
        }
    }
    else
    {
        // Parent: wait for the child
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            fprintf(stderr, " Child has returned with exit error (pid:%d)\n", getpid());
            return 1;
        }
    }

    fprintf(stderr, "LD_BIND_NOW was not found in %d\n", getpid());
    return (0);
}

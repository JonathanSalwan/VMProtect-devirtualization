/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        printf("Specify app name\n");
        return -1;
    }
    const char* childStr = "APPLICATION: After vfork in child\n";
    size_t childStrLen   = strlen(childStr);

    char* newArg[2];
    newArg[0] = (char*)argv[1];
    newArg[1] = NULL;

    pid_t child_id = vfork();
    if (child_id == 0)
    {
        // Use the write syscall directly.
        // Be careful not to use printf() which might allocate memory:
        // After vfork the child process shares the same memory space with the parent process.
        // So, allocating memory in the child process might mess with the parent process memory space!
        write(STDOUT_FILENO, (void*)childStr, childStrLen);
        execv(argv[1], newArg);
    }
    else
    {
        printf("APPLICATION: After vfork in parent\n");
        wait(0);
        execv(argv[1], newArg);
    }

    return 0;
}

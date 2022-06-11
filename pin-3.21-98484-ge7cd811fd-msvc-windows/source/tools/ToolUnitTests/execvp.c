/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

extern int errno;

extern** environ;

int main(int argc, char** argv)
{
    char* args[] = {"ls", argv[0], NULL};

    if (execvp("ls", args) == -1) perror("EXECV ERROR: exec failed");
}

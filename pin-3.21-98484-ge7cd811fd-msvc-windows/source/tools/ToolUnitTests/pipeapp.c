/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <unistd.h>

int main()
{
    int fd[2];

    if (pipe(fd) != 0)
    {
        fprintf(stderr, "pipe failed\n");
        return 1;
    }

    printf("Pipe returned file descriptors: %d, %d\n", fd[0], fd[1]);

    if (close(fd[0]) != 0)
    {
        fprintf(stderr, "File descriptor %d is invalid\n", fd[0]);
        return 1;
    }
    if (close(fd[1]) != 0)
    {
        fprintf(stderr, "File descriptor %d is invalid\n", fd[1]);
        return 1;
    }

    return 0;
}

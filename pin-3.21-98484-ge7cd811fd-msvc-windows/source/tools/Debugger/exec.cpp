/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Must specify a thing to exec\n");
        return 1;
    }

    execl(argv[1], argv[1], 0);
    printf("errno = %d\n", errno);
}

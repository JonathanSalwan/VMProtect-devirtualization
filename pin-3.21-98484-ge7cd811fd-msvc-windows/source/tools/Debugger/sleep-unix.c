/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

main()
{
    int fd[2];
    ssize_t sz;
    char buf[80];

    printf("This is pid %d\n", (int)getpid());

    pipe(fd);
    sz = read(fd[0], buf, sizeof(buf));

    printf("sz = %d\n", (int)sz);
    if (sz == -1) printf("errno = %d\n", errno);
    return 0;
}

/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <unistd.h>

int main()
{
    char* arg[] = {"randombadpath", 0};
    int res     = execve("/randombadpath", arg, 0);
    printf("Execve returned with %d\n", res);
    return 0;
}

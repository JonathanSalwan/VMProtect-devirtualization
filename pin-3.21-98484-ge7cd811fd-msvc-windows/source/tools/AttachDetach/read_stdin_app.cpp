/*
 * Copyright (C) 2016-2016 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <Windows.h>

int main()
{
    printf("my pid: %d\n", (int)GetCurrentProcessId());
    fflush(stdout);
    getchar();
    printf("App Success!\n");
    return 0;
}

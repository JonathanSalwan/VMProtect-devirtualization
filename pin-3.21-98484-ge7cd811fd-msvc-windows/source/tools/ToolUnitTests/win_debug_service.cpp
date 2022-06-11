/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <Windows.h>
#include <stdio.h>

// call OutputDebugString - Exercise windows debug service mechanism
int DebugService()
{
    int i = 0;
    for (int j = 0; j < 100; j++)
    {
        OutputDebugString("This is going to the debugger\n");
    }
    fprintf(stderr, "After OutputDebugString, i = %d\n", i);
    return 0;
}

/*------------------------  dispatcher ----------------------*/

int main()
{
    DebugService();
    return 0;
}
/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>

extern "C" int operImmCmds(int);

int main()
{
    // Call operImmCmds implemented in assembly
    int retVal = operImmCmds(42);
    printf("RetVal: %d\n", retVal);
    return 0;
}

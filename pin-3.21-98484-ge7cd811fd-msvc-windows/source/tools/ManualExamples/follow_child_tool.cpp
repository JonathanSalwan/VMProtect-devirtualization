/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

/* ===================================================================== */
/* Command line Switches */
/* ===================================================================== */

BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData)
{
    fprintf(stdout, "before child:%u\n", getpid());
    return TRUE;
}

/* ===================================================================== */

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    PIN_StartProgram();

    return 0;
}

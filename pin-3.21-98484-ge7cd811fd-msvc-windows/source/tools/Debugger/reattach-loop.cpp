/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

volatile int MyVariable = 89;
int Iterations          = 0;

static void Breakpoint();


int main()
{
    printf("Before the loop, MyVariable = %d\n", MyVariable);

    // The debugger changes 'MyVariable', which causes the loop to drop out.
    //
    while (MyVariable == 89)
    {
        Iterations++;
        Breakpoint();
    }

    printf("After the loop, MyVariable = %d\n", MyVariable);
    return 0;
}

static void Breakpoint()
{
    /* Debugger places a breakpoint here */
}

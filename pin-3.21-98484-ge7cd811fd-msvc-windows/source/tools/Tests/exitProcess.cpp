/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Demonstrate the usability of PIN_ExitProcess interface.
 */

#include <signal.h>
#include "pin_tests_util.H"

VOID gotSignal(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT* from, CONTEXT* to, INT32 info, VOID* v)
{
    if (info == SIGUSR1)
    {
        // the app is in an infinite loop at its exit
        PIN_ExitProcess(0);
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    PIN_AddContextChangeFunction(gotSignal, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

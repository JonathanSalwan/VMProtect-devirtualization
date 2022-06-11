/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
//  Sample usage:
//    pin -mt -t executeat_callback -- thread_wait

#include <stdio.h>
#include <iostream>
#include "pin.H"

PIN_LOCK pinLock;

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    // **** PIN_ExecuteAt() cannot be called from a callback!!!  ****
    // **** This is a test to ensure that an error is reported.  ****
    // **** Do not try this at home.                             ****

    PIN_ExecuteAt(ctxt);
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitLock(&pinLock);

    PIN_InitSymbols();

    PIN_Init(argc, argv);

    PIN_AddThreadStartFunction(ThreadStart, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

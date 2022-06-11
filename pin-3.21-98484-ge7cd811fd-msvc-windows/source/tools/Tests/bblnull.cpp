/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"

/* @file
 * Instrument every basic block with an empty analysis routine.
 * Use for timing measurements of bare instrumentation overhead.
 */

VOID BblRef()
{
    // nada
}

VOID Instruction(INS ins, VOID* v)
{
    if (INS_IsControlFlow(ins) || !INS_Valid(INS_Next(ins)))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)BblRef, IARG_END);
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);
    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

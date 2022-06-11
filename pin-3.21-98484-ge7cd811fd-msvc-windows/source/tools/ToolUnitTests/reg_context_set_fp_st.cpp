/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
#include "pin.H"

ADDRINT globSt0Val;

VOID AnalysisFunc(CONTEXT* context) { PIN_SetContextReg(context, REG_ST0, 0); }

VOID Instruction(INS ins, VOID* v) { INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalysisFunc, IARG_CONTEXT, IARG_END); }

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_StartProgram();

    return 0;
}

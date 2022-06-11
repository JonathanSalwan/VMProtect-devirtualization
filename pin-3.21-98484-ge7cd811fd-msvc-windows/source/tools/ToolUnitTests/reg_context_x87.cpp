/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
#include "pin.H"

KNOB< BOOL > KnobUseIargConstContext(KNOB_MODE_WRITEONCE, "pintool", "const_context", "0", "use IARG_CONST_CONTEXT");

ADDRINT globX87Val;

VOID AnalysisFunc(CONTEXT* context) { globX87Val = PIN_GetContextReg(context, REG_X87); }

VOID Instruction(INS ins, VOID* v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalysisFunc, (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT,
                   IARG_END);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_StartProgram();

    return 0;
}

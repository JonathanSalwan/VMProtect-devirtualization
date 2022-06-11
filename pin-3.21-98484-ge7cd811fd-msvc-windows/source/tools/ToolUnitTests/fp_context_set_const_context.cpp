/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
#include "pin.H"

#ifdef TARGET_IA32E
#define NUM_XMM_REGS 16
#else
#define NUM_XMM_REGS 8
#endif

VOID AnalysisFunc(CONTEXT* context)
{
    CHAR fpContextSpaceForFpConextFromPin[FPSTATE_SIZE];
    FPSTATE* fpContextFromPin = reinterpret_cast< FPSTATE* >(fpContextSpaceForFpConextFromPin);

    PIN_GetContextFPState(context, fpContextFromPin);

    for (int i = 0; i < NUM_XMM_REGS; i++)
    {
        fpContextFromPin->fxsave_legacy._xmms[i]._vec32[0] = 0xacdcacdc;
        fpContextFromPin->fxsave_legacy._xmms[i]._vec32[1] = 0xacdcacdc;
        fpContextFromPin->fxsave_legacy._xmms[i]._vec32[2] = 0xacdcacdc;
        fpContextFromPin->fxsave_legacy._xmms[i]._vec32[3] = 0xacdcacdc;
    }

    PIN_SetContextFPState(context, fpContextFromPin);
}

VOID Instruction(INS ins, VOID* v) { INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalysisFunc, IARG_CONST_CONTEXT, IARG_END); }

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_StartProgram();

    return 0;
}

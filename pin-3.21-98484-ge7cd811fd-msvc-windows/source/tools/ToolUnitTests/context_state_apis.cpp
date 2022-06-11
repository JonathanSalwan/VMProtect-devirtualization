/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include "pin.H"

extern "C" BOOL ProcessorSupportsAvx();

VOID CheckContextState(CONTEXT* ctxt)
{
    printf("context contains PROCESSOR_STATE_X87 %d\n", PIN_ContextContainsState(ctxt, PROCESSOR_STATE_X87));
    fflush(stdout);
    if (!PIN_ContextContainsState(ctxt, PROCESSOR_STATE_X87))
    {
        printf("***Error context does NOT contain PROCESSOR_STATE_X87\n");
        exit(-1);
    }
    printf("context contains PROCESSOR_STATE_XMM %d\n", PIN_ContextContainsState(ctxt, PROCESSOR_STATE_XMM));
    if (!PIN_ContextContainsState(ctxt, PROCESSOR_STATE_XMM))
    {
        printf("***Error context does NOT contain PROCESSOR_STATE_XMM\n");
        exit(-1);
    }
    printf("context contains PROCESSOR_STATE_YMM %d\n", PIN_ContextContainsState(ctxt, PROCESSOR_STATE_YMM));
    if (!PIN_ContextContainsState(ctxt, PROCESSOR_STATE_YMM) && ProcessorSupportsAvx())
    {
        printf("***Error context does NOT contain PROCESSOR_STATE_YMM\n");
        exit(-1);
    }
    if (PIN_ContextContainsState(ctxt, PROCESSOR_STATE_YMM) && !ProcessorSupportsAvx())
    {
        printf("***Error context contains PROCESSOR_STATE_YMM, but processor does NOT support Ymm\n");
        exit(-1);
    }
}

VOID Instruction(INS ins, VOID* v)
{
    static BOOL firstTime = true;
    if (firstTime)
    {
        firstTime = FALSE;
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckContextState, IARG_CONTEXT, IARG_END);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage() { return -1; }

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    printf("Pin supports PROCESSOR_STATE_X87 %d\n", PIN_SupportsProcessorState(PROCESSOR_STATE_X87));
    if (!PIN_SupportsProcessorState(PROCESSOR_STATE_X87))
    {
        printf("***Error Pin does NOT support PROCESSOR_STATE_X87\n");
        exit(-1);
    }
    printf("Pin supports PROCESSOR_STATE_XMM %d\n", PIN_SupportsProcessorState(PROCESSOR_STATE_XMM));
    if (!PIN_SupportsProcessorState(PROCESSOR_STATE_XMM))
    {
        printf("***Error Pin does NOT support PROCESSOR_STATE_XMM\n");
        exit(-1);
    }
    printf("Pin supports PROCESSOR_STATE_YMM %d\n", PIN_SupportsProcessorState(PROCESSOR_STATE_YMM));
    if (!PIN_SupportsProcessorState(PROCESSOR_STATE_YMM) && ProcessorSupportsAvx())
    {
        printf("***Error Pin does NOT support PROCESSOR_STATE_YMM\n");
        exit(-1);
    }
    if (PIN_SupportsProcessorState(PROCESSOR_STATE_YMM) && !ProcessorSupportsAvx())
    {
        printf("***Error Pin does support PROCESSOR_STATE_YMM, but processor does not\n");
        exit(-1);
    }
    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

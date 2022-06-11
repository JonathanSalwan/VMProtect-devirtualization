/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <stdio.h>
FILE* fp;

bool instrumented = FALSE;

unsigned int xmmInitVals[64];

extern "C" int SetXmmScratchesFun(unsigned int* values);

// Insert a call to an analysis routine that sets the scratch xmm registers, the call is inserted just after the
// vmovdqu instruction of LoadYmm0 (see ymm-asm-*.s)
static VOID InstrumentRoutine(RTN rtn, VOID*)
{
    if (RTN_Name(rtn) == "LoadYmm0")
    {
        RTN_Open(rtn);
        for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_Opcode(ins) == XED_ICLASS_VMOVDQU)
            {
                fprintf(fp, "instrumenting ins %p %s\n", (void*)INS_Address(ins), INS_Disassemble(ins).c_str());
                fflush(fp);
                instrumented = TRUE;
                INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)SetXmmScratchesFun, IARG_PTR, xmmInitVals, IARG_END);
            }
        }
        RTN_Close(rtn);
    }
}

static void OnExit(INT32, VOID*)
{
    if (!instrumented)
    {
        fprintf(fp, "***Error tool did not instrument the vmovdqu instruction of LoadYmm0\n");
        fflush(fp);
        PIN_ExitProcess(1);
    }
    else
    {
        fprintf(fp, "instrumented the vmovdqu instruction of LoadYmm0\n");
        fflush(fp);
    }
    fclose(fp);
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    fp = fopen("set_xmm_scratches_for_ymmtest.out", "w");

    // initialize memory area used to set values in ymm regs
    for (int i = 0; i < 64; i++)
    {
        xmmInitVals[i] = 0xdeadbeef;
    }

    PIN_InitSymbols();

    // Initialize pin
    PIN_Init(argc, argv);

    // Register Instruction to be called to instrument the vmovdqu instruction of LoadYmm0
    RTN_AddInstrumentFunction(InstrumentRoutine, 0);

    PIN_AddFiniFunction(OnExit, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

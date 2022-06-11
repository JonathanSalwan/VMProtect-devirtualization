/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
#include "pin.H"

/* Swizzle2 test rewritten to use the more modern interface to rewriting instruction addresses. */

UINT64 icount = 0;

#include "swizzle_util.h"

// When an image is loaded, check for a MyAlloc function
VOID Image(IMG img, VOID* v)
{
    //fprintf(stderr, "Loading %s\n",IMG_name(img));

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        //fprintf(stderr, "  sec %s\n", SEC_name(sec).c_str());
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            //fprintf(stderr, "    rtn %s\n", RTN_Name(rtn).c_str());
            // Swizzle the return value of MyAlloc

            if (RTN_Name(rtn) == "MyAlloc")
            {
                RTN_Open(rtn);

                fprintf(stderr, "Adding Swizzle to %s\n", "MyAlloc");
                RTN_InsertCall(rtn, IPOINT_AFTER, AFUNPTR(SwizzleRef), IARG_FUNCRET_EXITPOINT_REFERENCE, IARG_END);
                RTN_Close(rtn);
            }

            if (RTN_Name(rtn) == "MyFree")
            {
                RTN_Open(rtn);

                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(UnswizzleRef), IARG_FUNCARG_ENTRYPOINT_REFERENCE, 0, IARG_END);
                RTN_Close(rtn);
            }
        }
    }
}

KNOB< BOOL > KnobVerbose(KNOB_MODE_WRITEONCE, "pintool", "log", "0", "write a log file");

FILE* fout = 0;
VOID Instruction(INS ins, VOID* v)
{
    const char* d = INS_Disassemble(ins).c_str();
    if (fout)
    {
        fprintf(fout, "Checking: %s\n", d);
        fflush(fout);
    }

    int memops = INS_MemoryOperandCount(ins);

    for (int i = 0; i < memops; i++)
    {
        if (!INS_IsStandardMemop(ins) && !INS_HasMemoryVector(ins))
        {
            // We don't know how to treat these instructions
            return;
        }

        if (fout)
        {
            fprintf(fout, "REWRITING%d %s\n", i, d);
            fflush(fout);
        }

        INS_RewriteMemoryOperand(ins, i, REG(REG_INST_G0 + i));

        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(Unswizzle), IARG_MEMORYOP_EA, i, IARG_RETURN_REGS, REG_INST_G0 + i, IARG_END);
    }
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    if (KnobVerbose) fout = fopen("swizzle3.log", "w");

    INS_AddInstrumentFunction(Instruction, 0);
    IMG_AddInstrumentFunction(Image, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

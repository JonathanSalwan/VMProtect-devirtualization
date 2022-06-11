/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
#include "pin.H"

UINT64 icount = 0;

#include "swizzle_util.h"
using std::string;

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
    // Don't allow s to be deallocated before we've finished with d!
    string s      = hexstr(INS_Address(ins));
    const char* d = s.c_str();

    if (!INS_IsStandardMemop(ins) && !INS_HasMemoryVector(ins))
    {
        // We don't know how to treat these instructions
        return;
    }

    if (fout)
    {
        fprintf(fout, "Checking: %s %s\n", d, INS_Disassemble(ins).c_str());
        fflush(fout);
    }
    for (UINT32 memopIdx = 0; memopIdx < INS_MemoryOperandCount(ins); memopIdx++)
    {
        if (fout)
        {
            fprintf(fout, "REWRITING%d %s\n", memopIdx, d);
            fflush(fout);
        }
        REG scratchReg = REG(int(REG_INST_G0) + memopIdx);

        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(Unswizzle), IARG_MEMORYOP_EA, memopIdx, IARG_RETURN_REGS, scratchReg,
                       IARG_END);
        INS_RewriteMemoryOperand(ins, memopIdx, scratchReg);
    }
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    if (KnobVerbose) fout = fopen("swizzle2.log", "w");

    INS_AddInstrumentFunction(Instruction, 0);
    IMG_AddInstrumentFunction(Image, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

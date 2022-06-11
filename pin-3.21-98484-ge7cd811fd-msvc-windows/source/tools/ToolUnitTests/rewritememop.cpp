/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <signal.h>
#include <assert.h>
#include <stdio.h>
#include "pin.H"

FILE* trace = NULL;

ADDRINT ProcessAddress(ADDRINT val, VOID* ip)
{
    fprintf(trace, "%p: %p\n", ip, (void*)val);
    return val;
}

VOID RewriteIns(INS ins)
{
    //fprintf(stderr,"Rewriting %p\n",(void*)INS_Address(ins));

    for (UINT32 memopIdx = 0; memopIdx < INS_MemoryOperandCount(ins); memopIdx++)
    {
        REG scratchReg = REG(int(REG_INST_G0) + memopIdx);

        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(ProcessAddress), IARG_MEMORYOP_EA, memopIdx, IARG_INST_PTR, IARG_RETURN_REGS,
                       scratchReg, IARG_END);
        INS_RewriteMemoryOperand(ins, memopIdx, scratchReg);
    }
}

VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            RewriteIns(ins);
        }
    }
}

void AtEnd(INT32 code, VOID* arg) { fclose(trace); }

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    trace = fopen("rewritememtrace.out", "w");
    if (!trace)
    {
        perror("fopen");
        return 1;
    }

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(AtEnd, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

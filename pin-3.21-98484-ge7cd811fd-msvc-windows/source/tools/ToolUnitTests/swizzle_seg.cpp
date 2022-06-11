/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

ADDRINT newTLS     = 0;
ADDRINT newTLSSize = 0x80;

void AllocateNewTLS() { newTLS = (ADDRINT)malloc(newTLSSize); }

ADDRINT ProcessAddress(ADDRINT baseAddr, ADDRINT fullAddr, VOID* ip)
{
    ADDRINT displacement = fullAddr - baseAddr;
    return newTLS + displacement;
}

void Instruction(INS ins, VOID* v)
{
    RTN rtn = INS_Rtn(ins);
    if (RTN_Valid(rtn) && ((RTN_Name(rtn) == "SegAccessRtn") || (RTN_Name(rtn) == "SegAccessStrRtn")))
    {
        REG segReg = INS_SegmentRegPrefix(ins);

        if ((segReg != REG_SEG_GS) && (segReg != REG_SEG_FS)) return;

        REG baseReg = (segReg == REG_SEG_GS) ? REG_SEG_GS_BASE : REG_SEG_FS_BASE;

        for (UINT32 memopIdx = 0; memopIdx < INS_MemoryOperandCount(ins); memopIdx++)
        {
            REG scratchReg = REG(int(REG_INST_G0) + memopIdx);

            INS_RewriteMemoryOperand(ins, memopIdx, scratchReg);

            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(ProcessAddress), IARG_REG_VALUE, baseReg, IARG_MEMORYOP_EA, memopIdx,
                           IARG_INST_PTR, IARG_RETURN_REGS, scratchReg, IARG_END);
        }
    }
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    AllocateNewTLS();
    // Never returns
    PIN_StartProgram();

    return 0;
}

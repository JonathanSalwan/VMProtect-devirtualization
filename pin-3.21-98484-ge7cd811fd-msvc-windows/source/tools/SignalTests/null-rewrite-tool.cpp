/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool rewrites every memory instruction using Pin's INS_RewriteMemoryOperand() API,
 * but all instructions are rewritten to use their original effective addresses.
 */

#include <iostream>
#include <pin.H>

static void InstrumentIns(INS, VOID*);
static REG GetScratchReg(UINT32);
static ADDRINT GetMemAddress(ADDRINT);

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    INS_AddInstrumentFunction(InstrumentIns, 0);
    PIN_StartProgram();
    return 0;
}

static void InstrumentIns(INS ins, VOID*)
{
    for (UINT32 memIndex = 0; memIndex < INS_MemoryOperandCount(ins); memIndex++)
    {
        REG scratchReg = GetScratchReg(memIndex);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(GetMemAddress), IARG_MEMORYOP_EA, memIndex, IARG_RETURN_REGS, scratchReg,
                       IARG_END);
        INS_RewriteMemoryOperand(ins, memIndex, scratchReg);
    }
}

static REG GetScratchReg(UINT32 index)
{
    static std::vector< REG > regs;

    while (index >= regs.size())
    {
        REG reg = PIN_ClaimToolRegister();
        if (reg == REG_INVALID())
        {
            std::cerr << "*** Ran out of tool registers" << std::endl;
            PIN_ExitProcess(1);
            /* does not return */
        }
        regs.push_back(reg);
    }

    return regs[index];
}

static ADDRINT GetMemAddress(ADDRINT ea) { return ea; }

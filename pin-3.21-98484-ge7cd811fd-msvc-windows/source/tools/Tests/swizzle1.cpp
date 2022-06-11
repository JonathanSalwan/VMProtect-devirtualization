/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include "pin.H"

UINT64 icount = 0;

ADDRINT SwizzleAddress(ADDRINT val) { return val; }

VOID Instruction(INS ins, VOID* v)
{
    REG basereg = INS_dec(ins)->basereg;

    if (basereg == REG_INVALID()) return;

    // Not allowed to change esp
    if (basereg == REG_ESP) return;

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)SwizzleAddress, IARG_REG_VALUE, basereg, IARG_RETURN_REGS, basereg, IARG_END);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

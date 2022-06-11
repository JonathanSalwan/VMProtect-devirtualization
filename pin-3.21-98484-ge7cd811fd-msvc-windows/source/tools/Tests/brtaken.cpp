/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include "pin.H"
using std::endl;

UINT64 icount = 0;

VOID docount(INT32 c) { icount += c; }

VOID Instruction(INS ins, VOID* v)
{
    if (INS_IsControlFlow(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_BRANCH_TAKEN, IARG_END);
    }
}

VOID Fini(INT32 code, VOID* v) { std::cerr << "Count " << icount << endl; }

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

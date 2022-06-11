/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include "pin.H"
using std::endl;

UINT64 on  = 0;
UINT64 off = 0;

VOID docount(BOOL executing)
{
    if (executing)
    {
        on++; // predicated on
    }
    else
    {
        off++; // predicated off
    }
}

VOID Instruction(INS ins, VOID* v) { INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_EXECUTING, IARG_END); }

VOID Fini(INT32 code, VOID* v)
{
    std::cerr << "total: " << on + off << endl;
    std::cerr << "  predicated on:  " << on << endl;
    std::cerr << "  predicated off: " << off << endl;
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

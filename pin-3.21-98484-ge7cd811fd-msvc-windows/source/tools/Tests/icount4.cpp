/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include "pin.H"
using std::endl;

UINT64 on = 0;

VOID docount() { on++; }

VOID Instruction(INS ins, VOID* v) { INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END); }

VOID Fini(INT32 code, VOID* v) { std::cerr << "predicated on:  " << on << endl; }

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"
#include <iostream>

UINT64 icount = 0;

VOID docount()
{
#if defined(TARGET_IA32) && defined(TARGET_LINUX)
    asm("inc %%ebx" ::: "ebx");
#endif
}

VOID Instruction(INS ins, VOID* v) { INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END); }

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

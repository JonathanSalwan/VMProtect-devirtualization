/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * 1. Exit an application early
 * 2. Test for optimizer, see NeverCalled
 *
 */

#include <stdlib.h>
#include "pin.H"

VOID DoExit() { exit(0); }

/*
 * We pass 0 to this function to check if the optimizer properly generates this instruction:
 *
 * mov [0] = 0
 *
 */
VOID NeverCalled(int* v) { *v = 0; }

VOID Ins(INS ins, VOID* v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoExit), IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(NeverCalled), IARG_ADDRINT, 0, IARG_END);
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Ins, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

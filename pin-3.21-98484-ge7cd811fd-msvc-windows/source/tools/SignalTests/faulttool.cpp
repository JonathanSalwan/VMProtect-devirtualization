/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that Pin gives an error message if an inlined anlaysis
 * function causes a fault.
 */

#include "pin.H"

static VOID InstrumentInstruction(INS, VOID*);
static void OnInstruction(VOID*);

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(InstrumentInstruction, 0);

    PIN_StartProgram();
    return 0;
}

static VOID InstrumentInstruction(INS ins, VOID* v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(OnInstruction), IARG_PTR, (void*)16, IARG_END);
}

// The test assumes that OnInstruction is inline-able by pin.
static void OnInstruction(VOID* ptr)
{
    // Generate a fault.
    //
    *(int*)ptr = 0;
}

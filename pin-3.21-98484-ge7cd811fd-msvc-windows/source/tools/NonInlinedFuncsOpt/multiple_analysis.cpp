/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount = 0;

// This function is called before every instruction is executed
VOID docount() { icount++; }

BOOL firstInstrumentation = TRUE;
// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v)
{
    if (firstInstrumentation)
    {
        // Insert multiple calls to the docount function
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
        firstInstrumentation = FALSE;
    }
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return 1;

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This tool inserts instructions that set the status flags before each instruction.

#include "pin.H"

// The running count of instructions is kept here make it static to help the compiler optimize docount.
static UINT64 icount = 0;

// This function is called before every instruction, use fast linkage.
VOID PIN_FAST_ANALYSIS_CALL docount(ADDRINT c) { icount += c; }

VOID Instruction(INS ins, VOID* v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_FAST_ANALYSIS_CALL, IARG_UINT32, 1, IARG_END);
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

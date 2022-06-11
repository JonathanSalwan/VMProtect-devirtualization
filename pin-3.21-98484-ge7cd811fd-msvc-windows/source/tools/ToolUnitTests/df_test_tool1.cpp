/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"

#define DF_MASK 0x400
#define DF_POS 10

#if defined(__cplusplus)
extern "C"
#endif
    int numTimesDfIsSet = 0;

// This function is called before every instruction is executed
// it tests to see if DF is 0 as expected, if DF is set the value of
// numTimesDfIsSet is incremented by 1
#if defined(__cplusplus)
extern "C"
#endif
    VOID
    TestDfByReadFlags();

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v) { INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)TestDfByReadFlags, IARG_END); }

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    printf("Finished: numTimesDfIsSet %d\n", numTimesDfIsSet);
    fflush(stdout);
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

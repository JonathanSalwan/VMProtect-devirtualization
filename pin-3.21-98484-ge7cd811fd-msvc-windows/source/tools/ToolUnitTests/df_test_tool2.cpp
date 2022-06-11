/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"

#define DF_MASK 0x400
#define DF_POS 10
#if defined(__cplusplus)
extern "C"
#endif
    int source[4] = {1, 1, 1, 1};
#if defined(__cplusplus)
extern "C"
#endif
    int dest[4] = {0, 0, 0, 0};

// This function is called before every instruction is executed
// it should move source[1] and source[2] into dest[1] and dest[2] respectively
// Flags are written before the movsd
// If there is a bug and the DF is set - then it will  move source[1] and source[0] into dest[1]
// and dest[0] respectively
// In the Fini function we check that dest has it's expected values
#if defined(__cplusplus)
extern "C"
#endif
    VOID
    TestDfByMovsd();

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v) { INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)TestDfByMovsd, IARG_END); }

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    BOOL hadFailure = false;
    if (dest[0] != 0)
    {
        printf("Unexpected value of dest[0] %d\n", dest[0]);
        hadFailure = true;
    }
    if (dest[1] != 1)
    {
        printf("Unexpected value of dest[1] %d\n", dest[1]);
        hadFailure = true;
    }
    if (dest[2] != 1)
    {
        printf("Unexpected value of dest[2] %d\n", dest[2]);
        hadFailure = true;
    }
    if (dest[3] != 0)
    {
        printf("Unexpected value of dest[3] %d\n", dest[3]);
        hadFailure = true;
    }
    printf("Finished: hadFailure  %d\n", hadFailure);
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

/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

// Tool to check that AC flag is handeled correcty, by inserting misaligned read/write
// before or after selected instructions

int buff[8];

#if defined(__cplusplus)
extern "C"
#endif
    int
    GetFlags_asm();

VOID UnalignedReadAndWrite()
{
    char* unalignedPtr = (char*)buff;
    unalignedPtr++;
    int unalignedRead = *((int*)(unalignedPtr));
    unalignedPtr++;
    *((int*)(unalignedPtr)) = unalignedRead;
}

BOOL instrumentNext = FALSE;

VOID Instruction(INS ins, VOID* v)
{
    //printf ("Instruction at %x\n", INS_address(ins));
    if (INS_Opcode(ins) == XED_ICLASS_POPF || INS_Opcode(ins) == XED_ICLASS_POPFD || INS_Opcode(ins) == XED_ICLASS_POPFQ ||
        instrumentNext)
    {
        if (instrumentNext)
        {
            // IPOINT_BEFORE instrumentation on instruction after popf*
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)UnalignedReadAndWrite, IARG_END);
            instrumentNext = FALSE;
        }
        else
        { // IPOINT_AFTER instrumentation on popf*
            INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)UnalignedReadAndWrite, IARG_END);
            instrumentNext = TRUE;
        }
        return;
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v) {}

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

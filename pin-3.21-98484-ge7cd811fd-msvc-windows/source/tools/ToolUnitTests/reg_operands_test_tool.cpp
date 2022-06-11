/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"

#if defined(__cplusplus)
extern "C"
#endif
    int numFailures = 0;
int numFlagRegs     = 0;

// This function is called before every instruction is executed
// it tests to see if DF is 0 as expected, if DF is set the value of
// numTimesDfIsSet is incremented by 1

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v)
{
    const UINT32 max_r = INS_MaxNumRRegs(ins);

    for (UINT32 i = 0; i < max_r; i++)
    {
        const REG reg = INS_RegR(ins, i);
        if (reg == REG_STATUS_FLAGS || reg == REG_DF_FLAG)
        {
            printf("Error encountered unexpected flag reg at ins: %s\n", INS_Disassemble(ins).c_str());
            numFailures++;
        }
        else if (reg == REG_GFLAGS)
        {
            numFlagRegs++;
        }
    }

    const UINT32 max_w = INS_MaxNumWRegs(ins);

    for (UINT32 i = 0; i < max_w; i++)
    {
        const REG reg = INS_RegW(ins, i);
        if (reg == REG_STATUS_FLAGS || reg == REG_DF_FLAG)
        {
            printf("Error encountered unexpected flag reg at ins: %s\n", INS_Disassemble(ins).c_str());
            numFailures++;
        }
        else if (reg == REG_GFLAGS)
        {
            numFlagRegs++;
        }
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    if (numFailures)
    {
        printf("error numFailures %d\n", numFailures);
    }
    if (numFlagRegs == 0)
    {
        printf("error no flag regs found\n");
    }
    if (numFailures == 0 && numFlagRegs > 0)
    {
        printf("SUCCESS\n");
    }
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

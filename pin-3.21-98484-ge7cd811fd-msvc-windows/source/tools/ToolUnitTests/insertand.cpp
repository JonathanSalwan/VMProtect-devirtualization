/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test reproduced a bug in managing the flags. The problem occurs
 * when there are 2 inlined functions and the flags are partially live
 * through the first and dead in the second
 *
 *
 */

#include <iostream>
#include <cstdlib>
#include "pin.H"

extern "C" void incinst();
extern "C" void andinst();
extern "C" void leainst(void*);
extern "C" void leaindex(void*);
extern "C" void cmov_test(void*);

int cmov_data = 0;
char data[32];

void CheckData()
{
    if ((unsigned int)cmov_data != 0xbeefbeef) exit(1);

    if (data[16] != 2) exit(1);
}

long res;

VOID Instruction(INS ins, VOID* v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(incinst), IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(andinst), IARG_END);

    static bool first = true;
    if (first)
    {
        data[16] = 1;
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(cmov_test), IARG_PTR, &cmov_data, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(leainst), IARG_PTR, data, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CheckData), IARG_END);

        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(leaindex), IARG_REG_VALUE, REG_STACK_PTR, IARG_PTR, &res, IARG_END);
#if defined(TARGET_IA32E)
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(leaindex), IARG_REG_VALUE, REG_R9, IARG_PTR, &res, IARG_END);
#endif
    }

    first = false;
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

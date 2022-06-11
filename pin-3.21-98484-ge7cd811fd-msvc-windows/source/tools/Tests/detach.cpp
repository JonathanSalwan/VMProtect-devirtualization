/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"
#include <iostream>
using std::endl;

// This tool shows how to detach Pin from an
// application that is under Pin's control.

UINT64 icount = 0;
VOID docount()
{
    icount++;

    // Release control of application if 10000
    // instructions have been executed
    if ((icount % 10000) == 0)
    {
        PIN_Detach();
    }
}

VOID Instruction(INS ins, VOID* v) { INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END); }

VOID HelloWorld(VOID* v) { std::cerr << "Hello world!" << endl; }

VOID ByeWorld(VOID* v) { std::cerr << "Byebye world!" << endl; }

VOID Fini(INT32 code, VOID* v) { std::cerr << "Count: " << icount << endl; }

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    // Callback function to invoke for every
    // execution of an instruction
    INS_AddInstrumentFunction(Instruction, 0);

    // Callback functions to invoke before
    // Pin releases control of the application
    PIN_AddDetachFunction(HelloWorld, 0);
    PIN_AddDetachFunction(ByeWorld, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

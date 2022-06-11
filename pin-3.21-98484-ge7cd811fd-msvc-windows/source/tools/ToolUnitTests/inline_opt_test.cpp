/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
This tool is used, in conjuction with inline_opt_test_app, to verify that analysis
functions that are one basic block, but are too long to be inlined are recognized
as setting all the registers that they set
*/

#include "pin.H"

extern "C" void ZeroOutScratches();
extern "C" unsigned int scratchVals[];
unsigned int scratchVals[3];

VOID Instruction(INS ins, VOID* v) { INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(ZeroOutScratches), IARG_END); }

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

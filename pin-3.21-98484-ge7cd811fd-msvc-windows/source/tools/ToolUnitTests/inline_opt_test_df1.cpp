/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
This tool is used in conjuction with the inline_opt_test_df_app to verify that an analysis routine
that is 1 BBL but is too long to inline, that sets the DF flag is recognized by Pin as setting the DF
dlaf
*/
#include "pin.H"

extern "C" void SetDf();

VOID Instruction(INS ins, VOID* v) { INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SetDf), IARG_END); }

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

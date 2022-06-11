/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
Tool that requests IARG_CONST_CONTEXT at each instruction
*/
#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include "pin.H"
#include "instlib.H"

VOID GetSomeIntRegsFromContext(CONTEXT* ctxt)
{
    PIN_GetContextReg(ctxt, REG_INST_PTR);

    PIN_GetContextReg(ctxt, REG_GAX);

    PIN_GetContextReg(ctxt, REG_GBX);
}

VOID ReceiveContext(CONTEXT* ctxt) { GetSomeIntRegsFromContext(ctxt); }

VOID Instruction(INS ins, VOID* v) { INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ReceiveContext, IARG_CONST_CONTEXT, IARG_END); }

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

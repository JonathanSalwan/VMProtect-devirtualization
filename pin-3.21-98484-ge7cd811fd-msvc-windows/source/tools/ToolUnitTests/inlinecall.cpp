/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"

/*
  Demonstrates manually inlining a call to a simple function
*/

#if defined(TARGET_MAC)
static char const* replacedfunname = "_one";
#else
static char const* replacedfunname = "one";
#endif

ADDRINT oneAddress = 0;

ADDRINT Two()
{
    fprintf(stderr, "In 2\n");
    fflush(stderr);
    return 2;
}

VOID Image(IMG img, VOID*)
{
    RTN rtn = RTN_FindByName(img, replacedfunname);

    if (RTN_Valid(rtn))
    {
        fprintf(stderr, "Found one\n");
        fflush(stderr);
        oneAddress = RTN_Address(rtn);
    }
}

VOID Instruction(INS ins, VOID*)
{
    if (INS_IsDirectControlFlow(ins) && INS_DirectControlFlowTargetAddress(ins) == oneAddress)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(Two), IARG_RETURN_REGS, REG_GAX, IARG_END);
        INS_Delete(ins);
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(Image, 0);
    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

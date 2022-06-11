/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include "pin.H"
#include "instlib.H"

extern "C" void FpStack();

CONTEXT* globContext;
int numMmxsExecuted     = 0;
int numMmxsInstrumented = 0;

VOID AnalysisFunc(CONTEXT* ctxt)
{
    FpStack();
    globContext = ctxt;
    numMmxsExecuted++;
}

VOID Instruction(INS ins, VOID* v)
{
    xed_category_enum_t category  = (xed_category_enum_t)INS_Category(ins);
    xed_category_enum_t extension = (xed_category_enum_t)INS_Category(ins);
    if (category == XED_CATEGORY_MMX || category == XED_CATEGORY_3DNOW || extension == XED_EXTENSION_MMX ||
        category == XED_EXTENSION_3DNOW)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalysisFunc, IARG_CONST_CONTEXT, IARG_END);
        numMmxsInstrumented++;
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    if (numMmxsInstrumented == 0)
    {
        fprintf(stderr, "**Error No instrumented mmx instructions\n");
        PIN_ExitProcess(1);
    }
    if (numMmxsExecuted == 0)
    {
        fprintf(stderr, "**Error No executed mmx instructions\n");
        PIN_ExitProcess(1);
    }
}

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return -1;
    ;

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

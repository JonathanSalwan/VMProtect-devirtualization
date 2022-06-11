/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include "pin.H"
#include "instlib.H"

extern "C" int MMXSequence(unsigned int a, unsigned int b, unsigned int c, UINT64* aConcaTb);

CONTEXT* globContext;
int numFpsExecuted     = 0;
int numFpsInstrumented = 0;

VOID AnalysisFunc(CONTEXT* ctxt)
{
    UINT64 aConcaTb;
    MMXSequence(0xdeadbeef, 0xbaadf00d, 0xfeedf00d, &aConcaTb);
    globContext = ctxt;
    numFpsExecuted++;
}

VOID Instruction(INS ins, VOID* v)
{
    xed_iclass_enum_t iclass = static_cast< xed_iclass_enum_t >(INS_Opcode(ins));
    if (iclass == XED_ICLASS_FLD1 || iclass == XED_ICLASS_FADDP || iclass == XED_ICLASS_FSTP)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalysisFunc, IARG_CONST_CONTEXT, IARG_END);
        numFpsInstrumented++;
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    if (numFpsInstrumented == 0)
    {
        fprintf(stderr, "**Error No instrumented fp instructions\n");
        PIN_ExitProcess(1);
    }
    if (numFpsExecuted == 0)
    {
        fprintf(stderr, "**Error No executed fp instructions\n");
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

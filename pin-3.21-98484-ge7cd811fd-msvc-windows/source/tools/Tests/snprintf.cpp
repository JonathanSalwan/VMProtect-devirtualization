/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This tests the alignment of the stack on Intel64

#include "pin.H"
#include <fstream>
#include <stdio.h>

double fl;

VOID sn()
{
    char a[100];

    // snprintf does a movaps, which needs the stack aligned correctly
#if defined(PIN_GNU_COMPATIBLE) || defined(PIN_CRT)
    snprintf(a, sizeof(a) / sizeof(a[0]), "a %f\n", fl);
#elif defined(PIN_MS_COMPATIBLE)
    _snprintf(a, sizeof(a) / sizeof(a[0]), "a %f\n", fl);
#endif
}

VOID Instruction(INS ins, VOID* v)
{
    static BOOL first = true;

    if (!first) return;
    first = false;

    // test once with a context and once without
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(sn), IARG_CONTEXT, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(sn), IARG_END);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

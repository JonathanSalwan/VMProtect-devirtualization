/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"

FILE* out;

// Inspect the sequence, but do not instrument
VOID Trace(TRACE trace, VOID* v)
{
    fprintf(out, "Trace address %p\n", (CHAR*)(TRACE_Address(trace)));

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            fprintf(out, "  %p\n", (CHAR*)(INS_Address(ins)));
        }
    }

    fprintf(out, "\n");
}

int main(INT32 argc, CHAR** argv)
{
    out = fopen("sequence.out", "w");

    PIN_Init(argc, argv);

    TRACE_AddInstrumentFunction(Trace, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

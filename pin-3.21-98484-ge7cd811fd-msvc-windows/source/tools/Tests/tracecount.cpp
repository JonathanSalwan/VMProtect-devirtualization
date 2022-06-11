/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include "pin.H"
using std::endl;

UINT64 icount = 0;

VOID docount(INT32 c) { icount += c; }

VOID Trace(TRACE trace, VOID* v)
{
    // Instrument only at the head of the trace
    BBL bbl = TRACE_BblHead(trace);

    if (BBL_Valid(bbl))
    {
        INS_InsertCall(BBL_InsHead(bbl), IPOINT_BEFORE, (AFUNPTR)docount, IARG_UINT32, 1, IARG_END);
    }

    for (bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            ASSERTX(INS_Size(ins) != 0);
        }
    }
}

VOID Fini(INT32 code, VOID* v) { std::cerr << "Count: " << icount << endl; }

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Calculate number of traces with fall through using IARGLIST API.
 */

#include <stdio.h>
#include "pin.H"
#include <iostream>
using std::endl;

UINT64 allCount    = 0;
UINT64 fallThrough = 0;

VOID docount(BOOL hasFallThrough)
{
    allCount++;
    if (hasFallThrough) fallThrough++;
}

VOID Trace(TRACE trace, VOID* v)
{
    IARGLIST args = IARGLIST_Alloc();
    IARGLIST_AddArguments(args, IARG_BOOL, TRACE_HasFallThrough(trace), IARG_END);
    TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)docount, IARG_IARGLIST, args, IARG_END);
    IARGLIST_Free(args);
}

VOID Fini(INT32 code, VOID* v) { std::cerr << fallThrough << " traces out of " << allCount << " have fall-through" << endl; }

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

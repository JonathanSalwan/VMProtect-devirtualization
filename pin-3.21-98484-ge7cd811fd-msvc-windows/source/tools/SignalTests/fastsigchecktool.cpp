/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool is part of a test that verifies that Pin's "fast" signal handling
 * works.
 *
 * The test runs this tool on an application that delivers lots of signals.
 * The tool inserts a slow out-of-line analysis routine at every BBL,
 * so we expect a lot of the signals to be delivered while inside the analysis
 * function.  The test looks at internal statistics counters to verify that
 * the majority of these signals are delivered using the "fast" algorithm.
 * This validates that Pin's "current location" markers are set correctly, etc.
 */

#include <pin.H>

void OutOfLine()
{
    static volatile int Var = 0;

    for (int i = 0; i < 10; i++)
        Var++;
}

void InstrumentTrace(TRACE trace, VOID*)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
        BBL_InsertCall(bbl, IPOINT_BEFORE, AFUNPTR(OutOfLine), IARG_END);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    TRACE_AddInstrumentFunction(InstrumentTrace, 0);
    PIN_StartProgram();
    return 0;
}

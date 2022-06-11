/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"

// This tool causes one trace to be instrumented with so many calls that it will not
// fit into the default code-cache-block (which is 256K), even when Pin reduces the
// trace to 1 application instruction

ADDRINT addrOfTraceToInstrument = 0;
BOOL traceInstrumented          = FALSE;
const UINT32 numCallsToInsert   = 4000;
UINT32 numAnalysisCalls         = 0;

VOID AnalysisFunc(CONTEXT* ctxt) { numAnalysisCalls++; }

// Pin calls this function every time a new Trace is jitted
VOID Trace(TRACE trace, VOID* v)
{
    if (((!traceInstrumented) && TRACE_NumIns(trace) > 3) || addrOfTraceToInstrument == TRACE_Address(trace))
    {
        printf("Instrumenting Trace\n");
        addrOfTraceToInstrument = TRACE_Address(trace);
        traceInstrumented       = TRUE;
        for (UINT32 i = 0; i < numCallsToInsert; i++)
        {
            INS_InsertCall(BBL_InsHead(TRACE_BblHead(trace)), IPOINT_BEFORE, (AFUNPTR)AnalysisFunc, IARG_CONST_CONTEXT, IARG_END);
        }
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    ASSERTX(traceInstrumented);
    printf("numAnalysisCalls %d\n", numAnalysisCalls);
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    // Register Trace to be called to instrument the trace
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

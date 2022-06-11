/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool counts the number of SMC traces whose code has been modified
// by the running application
//

#include <stdio.h>
#include <iostream>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include "pin.H"
using std::endl;

// The running count of SMC traces
UINT64 smcCount = 0;

// This function is called before every trace is executed
VOID DoSmcCheck(VOID* traceAddr, VOID* traceCopyAddr, USIZE traceSize, CONTEXT* ctxP)
{
    if (memcmp(traceAddr, traceCopyAddr, traceSize) != 0)
    {
        smcCount++;
        free(traceCopyAddr);
        PIN_RemoveInstrumentationInRange((ADDRINT)traceAddr, (ADDRINT)traceAddr);
        PIN_ExecuteAt(ctxP);
    }
}

// Pin calls this function every time a new trace is encountered
VOID InstrumentTrace(TRACE trace, VOID* v)
{
    VOID* traceAddr;
    VOID* traceCopyAddr;
    USIZE traceSize;

    traceAddr = (VOID*)TRACE_Address(trace);

#if 0
    if (traceAddr < (void*)0xbf000000)
        return;
    fprintf(stderr,"Instrumenting trace at %p\n",traceAddr);
#endif

    traceSize     = TRACE_Size(trace);
    traceCopyAddr = malloc(traceSize);

    if (traceCopyAddr != 0)
    {
        memcpy(traceCopyAddr, traceAddr, traceSize);
        // Insert a call to DoSmcCheck before every trace
        TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)DoSmcCheck, IARG_PTR, traceAddr, IARG_PTR, traceCopyAddr, IARG_UINT32,
                         traceSize, IARG_CONTEXT, IARG_END);
    }
}

// This function is called when the application exits
// It prints the SMC trace counter
VOID Fini(INT32 code, VOID* v) { std::cerr << "SMC Count = " << smcCount << endl; }

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    // Register InstrumentTrace function
    TRACE_AddInstrumentFunction(InstrumentTrace, 0);

    // Register application exit call back
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

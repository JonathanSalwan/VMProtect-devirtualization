/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool calls PIN_ExitApplication.
 * It can be used with any executable, since it just forces exit and checks that
 * relevant callbacks have occurred.
 */
#include <stdio.h>
#include "pin.H"
using std::string;

static KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "exitapplication.out", "specify file name");
static KNOB< BOOL > KnobCallback(KNOB_MODE_WRITEONCE, "pintool", "c", "0", "exit from a callback");

static FILE* out;

static VOID ThreadStart(THREADID threadIndex, CONTEXT* ctxt, INT32 flags, VOID* v) { PIN_ExitApplication(0); }

static VOID ThreadFini(THREADID tid, CONTEXT const* c, INT32 code, VOID* v)
{
    fprintf(out, "Thread Fini callback for thread %d\n", tid);
}

static VOID Fini(INT32 code, VOID* v)
{
    fprintf(out, "Process Fini callback\n");
    fclose(out);
}

static VOID MakeExitCallback() { PIN_ExitApplication(0); }

static VOID InstrumentTrace(TRACE t, VOID* v) { TRACE_InsertCall(t, IPOINT_BEFORE, (AFUNPTR)MakeExitCallback, IARG_END); }

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    out = fopen(KnobOutputFile.Value().c_str(), "w");

    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_AddFiniFunction(Fini, 0);

    if (KnobCallback)
    {
        PIN_AddThreadStartFunction(ThreadStart, 0);
    }
    else
    {
        TRACE_AddInstrumentFunction(InstrumentTrace, 0);
    }

    // Never returns
    PIN_StartProgram();

    return 0;
}

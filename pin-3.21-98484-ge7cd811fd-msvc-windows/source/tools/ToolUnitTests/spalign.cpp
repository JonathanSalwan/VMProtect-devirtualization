/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test is specific to the Intel(R) 64 architecture.  It verifies that the
 * stack is propertly aligned in analysis routines and call-backs.
 */

#include "pin.H"

extern "C" VOID CheckSPAlign();

static VOID InstrumentTrace(TRACE trace, VOID* v);
static VOID AtTraceOutOfLine();
static ADDRINT AtTraceIf();
static VOID AtEnd(INT32 code, VOID* v);

int main(INT32 argc, CHAR** argv)
{
    CheckSPAlign();
    PIN_Init(argc, argv);

    TRACE_AddInstrumentFunction(InstrumentTrace, 0);
    PIN_AddFiniFunction(AtEnd, 0);

    PIN_StartProgram();
    return 0;
}

static VOID InstrumentTrace(TRACE trace, VOID* v)
{
    CheckSPAlign();

    static int testNum = 0;
    switch (testNum++)
    {
        case 0:
            // Test an out-of-line analysis call.
            //
            TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(AtTraceOutOfLine), IARG_END);
            break;

        case 1:
            // Test an out-of-line "if/then" call.
            //
            TRACE_InsertIfCall(trace, IPOINT_BEFORE, AFUNPTR(AtTraceIf), IARG_END);
            TRACE_InsertThenCall(trace, IPOINT_BEFORE, AFUNPTR(AtTraceOutOfLine), IARG_END);
            break;

        case 2:
            // Test an inlined analysis call.
            //
            TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(CheckSPAlign), IARG_END);
            break;

        case 3:
            // Test an "if/then" call where the "then" is inlined.
            //
            TRACE_InsertIfCall(trace, IPOINT_BEFORE, AFUNPTR(AtTraceIf), IARG_END);
            TRACE_InsertThenCall(trace, IPOINT_BEFORE, AFUNPTR(CheckSPAlign), IARG_END);
            break;

        case 4:
            // Test an out-of-line analysis call with context argument
            //
            TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(AtTraceOutOfLine), IARG_CONTEXT, IARG_END);
            break;

        case 5:
            // Test an out-of-line "if/then" call with context argument
            //
            TRACE_InsertIfCall(trace, IPOINT_BEFORE, AFUNPTR(AtTraceIf), IARG_END);
            TRACE_InsertThenCall(trace, IPOINT_BEFORE, AFUNPTR(AtTraceOutOfLine), IARG_CONTEXT, IARG_END);
            break;

        default:
            testNum = 0;
            break;
    }
}

static VOID AtTraceOutOfLine() { CheckSPAlign(); }

static ADDRINT AtTraceIf()
{
    CheckSPAlign();
    return 1;
}

static VOID AtEnd(INT32 code, VOID* v) { CheckSPAlign(); }

/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include <assert.h>
#include "pin.H"

#include "instrumentation_order_app.h"
using std::endl;
using std::ofstream;
using std::string;

// A knob for defining the output file name
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "instrumentation_order15.out",
                              "specify file name for instrumentation order output");

// ofstream object for handling the output.
ofstream outstream;

void Emit(char const* message) { outstream << message << endl; }

static VOID Trace(TRACE trace, VOID* v)
{
    RTN rtn = TRACE_Rtn(trace);

    if (!RTN_Valid(rtn) || RTN_Name(rtn) != watch_rtn)
    {
        return;
    }

    if (TRACE_Address(trace) == RTN_Address(rtn))
    {
        // Pin does not support issuing an RTN_InsertCall from the TRACE instrumentation callback
        // This will cause Pin to terminate with an error
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(Emit), IARG_PTR, "Ins instrumentation1", IARG_END);
        RTN_Close(rtn);
    }
}

static VOID Fini(INT32 code, VOID* v) { outstream.close(); }

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    outstream.open(KnobOutputFile.Value().c_str());

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

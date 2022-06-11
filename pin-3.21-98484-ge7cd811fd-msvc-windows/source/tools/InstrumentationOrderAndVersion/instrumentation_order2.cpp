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
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "instrumentation_order2.out",
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
        TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(Emit), IARG_PTR, "Trace instrumentation", IARG_CALL_ORDER,
                         CALL_ORDER_FIRST + 2, IARG_END);
        printf("Trace Instrumenting %s\n", watch_rtn);
    }
}

static VOID Ins(INS ins, VOID* v)
{
    RTN rtn = INS_Rtn(ins);

    if (!RTN_Valid(rtn) || RTN_Name(rtn) != watch_rtn)
    {
        return;
    }

    if (INS_Address(ins) == RTN_Address(rtn))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(Emit), IARG_PTR, "Ins instrumentation", IARG_CALL_ORDER, CALL_ORDER_FIRST + 3,
                       IARG_END);
        printf("Ins Instrumenting %s\n", watch_rtn);
    }
}

static VOID Rtn(RTN rtn, VOID* v)
{
    if (!RTN_Valid(rtn) || RTN_Name(rtn) != watch_rtn)
    {
        return;
    }
    printf("Rtn Instrumenting %s\n", watch_rtn);
    RTN_Open(rtn);

    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(Emit), IARG_PTR, "RTN instrumentation", IARG_CALL_ORDER, CALL_ORDER_FIRST + 1,
                   IARG_END);
    RTN_Close(rtn);
}

static VOID Image(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, watch_rtn);

    if (!RTN_Valid(rtn))
    {
        return;
    }
    printf("Image Instrumenting %s\n", watch_rtn);
    RTN_Open(rtn);

    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(Emit), IARG_PTR, "IMG instrumentation", IARG_CALL_ORDER, CALL_ORDER_FIRST,
                   IARG_END);
    RTN_Close(rtn);
}

static VOID Fini(INT32 code, VOID* v) { outstream.close(); }

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    outstream.open(KnobOutputFile.Value().c_str());

    TRACE_AddInstrumentFunction(Trace, 0);

    INS_AddInstrumentFunction(Ins, 0);

    RTN_AddInstrumentFunction(Rtn, 0);

    IMG_AddInstrumentFunction(Image, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

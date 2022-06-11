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
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "instrumentation_order16.out",
                              "specify file name for instrumentation order output");

// ofstream object for handling the output.
ofstream outstream;

void Emit(char const* message) { outstream << message << endl; }

static VOID Image(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, watch_rtn);

    if (!RTN_Valid(rtn))
    {
        return;
    }
    printf("Image Instrumenting %s\n", watch_rtn);
    RTN_Open(rtn);
    INS ins = RTN_InsHeadOnly(rtn);
    ASSERTX(INS_Valid(ins));

    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(Emit), IARG_PTR, "IMG instrumentation1", IARG_CALL_ORDER, CALL_ORDER_FIRST + 2,
                   IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(Emit), IARG_PTR, "IMG instrumentation2", IARG_CALL_ORDER, CALL_ORDER_FIRST + 2,
                   IARG_END);
    RTN_Close(rtn);
}

static VOID Fini(INT32 code, VOID* v) { outstream.close(); }

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    outstream.open(KnobOutputFile.Value().c_str());

    IMG_AddInstrumentFunction(Image, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

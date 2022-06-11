/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool prints a trace of image load and unload events
//

#include "pin.H"
#include <stdio.h>

using std::string;
FILE* trace;

static KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "imageload.out", "Output file");

// Pin calls this function every time a new img is loaded
VOID ImageLoad(IMG img, VOID* v)
{
    SEC sec;
    RTN rtn;

    if (string::npos == IMG_Name(img).find("dummy_dll")) return;

    fprintf(trace, "library: %s\n", IMG_Name(img).c_str());

    for (sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
        for (rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
            fprintf(trace, "rtn: %s\n", RTN_Name(rtn).c_str());

    fflush(trace);
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize symbol processing in exports only mode.
    // External symbol server is not used
    PIN_InitSymbolsAlt(EXPORT_SYMBOLS);

    // Initialize pin
    if (PIN_Init(argc, argv) != 0)
    {
        return 1;
    }

    trace = fopen(KnobOutputFile.Value().c_str(), "w");

    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Start the program, never returns
    if (PIN_IsProbeMode())
    {
        fprintf(trace, "Probe mode\n");
        PIN_StartProgramProbed();
    }
    else
    {
        fprintf(trace, "JIT mode\n");
        PIN_StartProgram();
    }

    return 0;
}

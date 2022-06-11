/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool prints a trace of image load and unload events
//

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdio.h>

using std::cout;
using std::endl;
using std::ofstream;
using std::string;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "load_dummy.out", "specify file name");

ofstream TraceFile;

// Pin calls this function every time a new img is loaded
VOID ImageLoad(IMG img, VOID* v) { TraceFile << IMG_Name(img).c_str() << " loaded" << endl; }

// Pin calls this function every time an img is unloaded
VOID ImageUnload(IMG img, VOID* v)
{
    TraceFile << IMG_Name(img).c_str() << " unloaded" << endl;
    ;
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize pin
    if (PIN_Init(argc, argv) != 0)
    {
        return 1;
    }

    TraceFile.open(KnobOutputFile.Value().c_str());

    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register ImageUnload to be called when an image is unloaded
    IMG_AddUnloadFunction(ImageUnload, 0);

    // Start the program, never returns
    if (PIN_IsProbeMode())
    {
        PIN_StartProgramProbed();
    }

    else
    {
        PIN_StartProgram();
    }

    return 0;
}

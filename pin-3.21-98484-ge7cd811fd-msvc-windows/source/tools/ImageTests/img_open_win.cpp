/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool validates Pin handling of rebased image
// and verifies that IMG_Open API operates properly.
//

#include "pin.H"
#include <stdio.h>
#include <iostream>
#include <fstream>

using std::cout;
using std::endl;
using std::ofstream;
using std::string;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "imag_open_win.out", "specify file name");

ofstream TraceFile;

// Pin calls this function every time a new img is loaded
VOID ImageLoad(IMG img, VOID* v)
{
    SEC sec;
    RTN rtn;

    if (string::npos == IMG_Name(img).find("dummy_dll")) return;

    TraceFile << "library: " << IMG_Name(img) << endl;

    for (sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
        for (rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
            TraceFile << "rtn: " << RTN_Name(rtn) << endl;
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

    // Test IMG_Open
    IMG img = IMG_Open("dummy_dll.dll");
    if (IMG_Valid(img))
    {
        ImageLoad(img, NULL);
    }
    IMG_Close(img);

    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);

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

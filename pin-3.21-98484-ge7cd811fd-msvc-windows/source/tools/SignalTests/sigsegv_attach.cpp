/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  Test that SIGSEGV signal is handled correctly. This tool lets the app know we are attached.
 *  The tool should be used with the sigsegv_app application.
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::endl;
using std::ofstream;

// Global variables

ofstream outFile; // The tool's output file for printing the loaded images.

bool attached = false;

static void OnDoRelease(ADDRINT doRelease)
{
    if (!attached) return;
    *((bool*)doRelease) = true;
}

static VOID ImageLoad(IMG img, VOID* v)
{
    outFile << IMG_Name(img) << endl;
    if (IMG_IsMainExecutable(img))
    {
        RTN doReleaseRtn = RTN_FindByName(img, "DoRelease");
        ASSERTX(RTN_Valid(doReleaseRtn));

        RTN_Open(doReleaseRtn);
        RTN_InsertCall(doReleaseRtn, IPOINT_BEFORE, AFUNPTR(OnDoRelease), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        RTN_Close(doReleaseRtn);
    }
}

static VOID OnAppStart(VOID* v) { attached = true; }

int main(INT32 argc, CHAR* argv[])
{
    // Initialization.
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    // Add instrumentation.
    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddApplicationStartFunction(OnAppStart, 0);

    // Start the program.
    PIN_StartProgram(); // never returns

    return 0;
}

/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This tool verifies that partial unmapping of an image does not result in an image unload callback.

#include <stdio.h>
#include "pin.H"
#include <fstream>
using std::ofstream;
using std::string;

using std::endl;
using std::flush;

/////////////////////
// GLOBAL VARIABLES
/////////////////////

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "badmunmap.out", "specify output file name");
ofstream OutFile;
bool markerHit   = false;
bool testSuccess = false;

INT32 Usage()
{
    OutFile << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

void NotifyMarker()
{
    OutFile << "TOOL: AppMarker has been called" << endl << flush;
    markerHit = true;
}

VOID ImageUnload(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        OutFile << "TOOL: Calling ImageUnload for " << IMG_Name(img) << endl << flush;

        if (!markerHit)
        {
            OutFile << "TOOL: ERROR: ImageUnload called before the marker was hit" << endl << flush;
        }
        else
        {
            testSuccess = true;
        }
    }
}

VOID Routine(RTN rtn, VOID* v)
{
    if (RTN_Name(rtn) == "AppMarker" || RTN_Name(rtn) == "_AppMarker")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, NotifyMarker, IARG_END);
        RTN_Close(rtn);
    }
}

VOID Fini(int, VOID* v)
{
    OutFile << "TOOL: Calling Fini" << endl << flush;
    if (testSuccess)
    {
        OutFile << "TOOL: SUCCESS" << endl << flush;
    }
    else
    {
        OutFile << "TOOL: FAILURE" << endl << flush;
    }
}

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    IMG_AddUnloadFunction(ImageUnload, 0);
    RTN_AddInstrumentFunction(Routine, 0);
    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram(); // never returns

    return 0;
}

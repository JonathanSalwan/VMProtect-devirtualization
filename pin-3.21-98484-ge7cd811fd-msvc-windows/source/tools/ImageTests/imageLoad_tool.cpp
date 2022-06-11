/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool verifies that the registered image callback is being called for images that are loaded after attaching Pin in probe mode.
 */

#include "pin.H"
#include <fstream>
using std::endl;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "imageLoad.out", "specify file name");

ofstream TraceFile;
/* ===================================================================== */

int afterAttachProbe(void) { return 1; }

static VOID imageLoad(IMG img, VOID* v)
{
    TraceFile << "in image callback of image: " << IMG_Name(img).c_str() << endl;
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_FindByName(img, "AfterAttach");
        if (RTN_Valid(rtn))
        {
            RTN_ReplaceProbed(rtn, AFUNPTR(afterAttachProbe));
        }
    }
}
int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    // Initialize pin
    PIN_Init(argc, argv);

    TraceFile.open(KnobOutputFile.Value().c_str());
    IMG_AddInstrumentFunction(imageLoad, 0);

    // Start the program, never returns
    PIN_StartProgramProbed();

    return 0;
}

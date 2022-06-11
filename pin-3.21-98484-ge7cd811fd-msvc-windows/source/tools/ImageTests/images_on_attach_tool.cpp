/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  This tool is used to verify that all image-load callbacks are called when Pin attaches to a running program.
 *  The test checks for images that were already loaded before Pin attached as well as images loaded after attach.
 *  The tool should be used with the images_on_attach_app application.
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::endl;
using std::ofstream;
using std::string;

// Global variables

// A knob for defining the file with list of loaded images
KNOB< string > KnobImagesLog(KNOB_MODE_WRITEONCE, "pintool", "o", "images_on_attach.log", "log file for the tool");

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
        if (!RTN_Valid(doReleaseRtn))
        {
            cerr << "TOOL ERROR: Unable to find the DoRelease function in the application." << endl;
            PIN_ExitProcess(1);
        }
        RTN_Open(doReleaseRtn);
        RTN_InsertCall(doReleaseRtn, IPOINT_BEFORE, AFUNPTR(OnDoRelease), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        RTN_Close(doReleaseRtn);
    }
}

static VOID OnAppStart(VOID* v) { attached = true; }

static VOID Fini(INT32 code, VOID* v) { outFile.close(); }

int main(INT32 argc, CHAR* argv[])
{
    // Initialization.
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    // Open the tool's output file for printing the loaded images.
    outFile.open(KnobImagesLog.Value().c_str());
    if (!outFile.is_open() || outFile.fail())
    {
        cerr << "TOOL ERROR: Unable to open the images file." << endl;
        PIN_ExitProcess(1);
    }

    // Add instrumentation.
    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddApplicationStartFunction(OnAppStart, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Start the program.
    PIN_StartProgram(); // never returns

    return 0;
}

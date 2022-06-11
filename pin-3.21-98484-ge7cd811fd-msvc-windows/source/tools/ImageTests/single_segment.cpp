/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool prints a trace of image load and unload events
//

#include <stdio.h>
#include "pin.H"

BOOL ImageEntryWasCalled = FALSE;

VOID MainImageEntry() { ImageEntryWasCalled = TRUE; }

// Pin calls this function every time a new img is loaded
// It can instrument the image, but this example does not
// Note that imgs (including shared libraries) are loaded lazily

VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsVDSO(img))
    {
        return;
    }
    RTN rtn = RTN_FindByName(img, "_start");
    ASSERTX(RTN_Valid(rtn));
    RTN_Open(rtn);
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)MainImageEntry, IARG_END);
    RTN_Close(rtn);
}

VOID Fini(INT32 code, VOID* v)
{
    if (0 == code && !ImageEntryWasCalled)
    {
        fprintf(stderr, "MainImageEntry() was not called!\n");
        PIN_ExitApplication(-1);
    }
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize pin
    PIN_Init(argc, argv);

    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Starts the program - never return
    PIN_StartProgram();

    return 0;
}

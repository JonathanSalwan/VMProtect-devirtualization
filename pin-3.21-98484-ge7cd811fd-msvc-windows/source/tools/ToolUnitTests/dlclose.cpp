/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool prints a trace of image load and unload events
//

#include <stdio.h>
#include "pin.H"

#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>

using std::cout;
using std::endl;

BOOL inMain = FALSE;

void MainBefore() { inMain = TRUE; }

VOID MainAfter() { inMain = FALSE; }

VOID ImageLoad(IMG img, VOID* v)
{
    if (inMain)
    {
        cout << "Loaded " << IMG_Name(img) << endl;
    }

    if (IMG_IsMainExecutable(img))
    {
        RTN mainRtn = RTN_FindByName(img, "_main");
        if (!RTN_Valid(mainRtn)) mainRtn = RTN_FindByName(img, "main");

        if (!RTN_Valid(mainRtn))
        {
            cout << "Can't find the main routine in " << IMG_Name(img) << endl;
            exit(1);
        }
        RTN_Open(mainRtn);
        RTN_InsertCall(mainRtn, IPOINT_BEFORE, AFUNPTR(MainBefore), IARG_END);
        RTN_InsertCall(mainRtn, IPOINT_AFTER, AFUNPTR(MainAfter), IARG_END);
        RTN_Close(mainRtn);
    }
}

VOID ImageUnload(IMG img, VOID* v)
{
    if (inMain)
    {
        cout << "Unloaded " << IMG_Name(img) << endl;
    }
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    // Initialize pin
    PIN_Init(argc, argv);

    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register ImageUnload to be called when an image is unloaded
    IMG_AddUnloadFunction(ImageUnload, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool is a negative test, it verify that pin report error on
// using illegal callbacks in probe/jit mode.
//

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

VOID ImageLoad(IMG img, VOID* v)
{
    printf("Loading %s, Image id = %d\n", IMG_Name(img).c_str(), IMG_Id(img));
    fflush(stdout);
}

// Pin calls this function every time a new img is unloaded
// You can't instrument an image that is about to be unloaded
VOID ImageUnload(IMG img, VOID* v)
{
    printf("Unloading %s\n", IMG_Name(img).c_str());
    fflush(stdout);
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    printf("Got thread start notification\n");
    fflush(stdout);
}

VOID Fini(INT32 code, VOID* v)
{
    printf("We are not expecting to get here...\n");
    fflush(stdout);
    exit(1);
}

VOID DetachComplete(VOID* v)
{
    printf("Detach complete\n");
    fflush(stdout);
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

    // Register ImageUnload to be called when an image is unloaded
    IMG_AddUnloadFunction(ImageUnload, 0);

    // Register fini funcation
    PIN_AddFiniFunction(Fini, 0);

    printf("This is a negative test it is expected to fail\n");

    // Never returns
    if (PIN_IsProbeMode())
    {
        // Thread start callback is JIT mode only
        PIN_AddThreadStartFunction(ThreadStart, 0);
        PIN_StartProgramProbed();
    }
    else
    {
        // Detach probed callback is Probe mode only
        PIN_AddDetachFunctionProbed(DetachComplete, 0);
        PIN_StartProgram();
    }

    return 0;
}

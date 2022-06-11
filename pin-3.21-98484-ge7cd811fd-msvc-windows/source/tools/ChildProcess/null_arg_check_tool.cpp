/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include "arglist.h"
#include <stdio.h>

/* ===================================================================== */
/* Command line Switches */
/* ===================================================================== */

BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData) { return TRUE; }

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    BOOL jitMode = (v == 0);
    if (IMG_IsMainExecutable(img))
    {
        fprintf(stdout, "Image %s is loaded in %s mode\n", IMG_Name(img).c_str(), (jitMode ? "JIT" : "PROBE"));
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    // Never returns
    if (PIN_IsProbeMode())
    {
        IMG_AddInstrumentFunction(ImageLoad, (VOID*)1);
        PIN_StartProgramProbed();
    }
    else
    {
        IMG_AddInstrumentFunction(ImageLoad, 0);
        PIN_StartProgram();
    }
    return 0;
}

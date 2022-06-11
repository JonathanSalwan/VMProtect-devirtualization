/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/**
 * @file
 * see check_xmms_app.c for details
 */
#include "pin.H"
#include <stdio.h>
#include "tool_macros.h"

extern "C" int ScrambleXmms() ASMNAME("ScrambleXmms");

#if (defined TARGET_WINDOWS) && (defined(TARGET_IA32))
// windows 32 bits add underscore prefix and RTN_FindByName() can't find the name without the prefix although for regular
// binaries it can. It seems the if an assembly function contains the function, then RTN_FindByName() won't be able to find it.
#define DONOTHING "_DoNothing"
#else
#define DONOTHING "DoNothing"
#endif

VOID ScrambleXmmsInsideAnalysisOnly()
{
    ScrambleXmms();
    PIN_Detach();
}

VOID Image(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, DONOTHING);
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);

        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)ScrambleXmmsInsideAnalysisOnly, IARG_END);

        RTN_Close(rtn);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    // Detach didn't occur - bug
    assert(0);
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    // Initialize pin
    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(Image, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

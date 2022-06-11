/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This Pin tool prints full name of the tool DLL file
 *  using Pin client API PIN_ToolFullPath().
 *  The printed name used as pattern of grep in test
 *  to validate correctness of value returned by the API.
 */

#include "pin.H"
#include <iostream>
using std::cerr;
using std::flush;

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return 1;
    }

    const char* toolName = PIN_ToolFullPath();
    cerr << toolName << flush;

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
This tool is used to output that the child process was created in the correct mode 
(probe or jit)
It does PIN_InitSymbols
*/
#include "pin.H"
#include <iostream>

using std::cout;
using std::endl;

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    cout << "In follow_child with_sym PinTool is probed " << decstr(PIN_IsProbeMode()) << endl;

    // Never returns
    if (PIN_IsProbeMode())
    {
        PIN_StartProgramProbed();
    }
    else
    {
        PIN_StartProgram();
    }

    return 0;
}

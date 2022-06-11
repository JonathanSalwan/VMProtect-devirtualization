/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * 1. compile with debug info- IMG_hasLinesData returns true
 * 2. strip debug info from executable(linux)/compile without debug (macOS* & windows)- IMG_hasLinesData returns false
 */

#include "pin.H"
#include <assert.h>
#include <iostream>
#include <fstream>
using std::string;

KNOB< string > KnobCases(KNOB_MODE_WRITEONCE, "pintool", "state", "debug",
                         "specify test behavior according to state. debug- debug info exist. no_debug - not exist ");

BOOL hasLineInfoUsingGetSourceLocation(IMG img)
{
    string filename;

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            INT32 line = 0;
            RTN_Open(rtn);
            PIN_GetSourceLocation(RTN_Address(rtn), NULL, &line, &filename);
            if (0 != line)
            {
                RTN_Close(rtn);
                return TRUE;
            }
            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                line = 0;
                PIN_GetSourceLocation(INS_Address(ins), NULL, &line, &filename);
                if (0 != line)
                {
                    RTN_Close(rtn);
                    return TRUE;
                }
            }
            RTN_Close(rtn);
        }
    }
    return FALSE;
}

VOID ImageLoad(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img)) return;

    if ("debug" == KnobCases.Value())
    {
        ASSERTX(TRUE == hasLineInfoUsingGetSourceLocation(img));
        ASSERTX(TRUE == IMG_hasLinesData(img));
    }
    if ("no_debug" == KnobCases.Value())
    {
        ASSERTX(FALSE == hasLineInfoUsingGetSourceLocation(img));
        ASSERTX(FALSE == IMG_hasLinesData(img));
    }
}

int main(int argc, char* argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize pin
    PIN_Init(argc, argv);
    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_StartProgram();
    return -3;
}

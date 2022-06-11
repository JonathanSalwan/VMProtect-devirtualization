/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
  This tool is used for testing the "DEBUGGER_ERROR" error code.
*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>

#ifdef TARGET_MAC
#define BAR_FN_NAME "_Bar2"
#else
#define BAR_FN_NAME "Bar2"
#endif

/* ===================================================================== */
void Before() { PIN_ERROR_DEBUGGER("Debugger error"); }

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            if (RTN_Name(rtn) == BAR_FN_NAME)
            {
                RTN_Open(rtn);
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(Before), IARG_END);
                RTN_Close(rtn);
            }
        }
    }
}

/* ===================================================================== */
int main(INT32 argc, CHAR* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

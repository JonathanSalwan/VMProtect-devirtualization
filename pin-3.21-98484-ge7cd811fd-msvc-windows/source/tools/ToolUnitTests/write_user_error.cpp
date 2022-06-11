/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
  Replace an original function with a custom function defined in the tool. The
  new function can have either the same or different signature from that of its
  original function.
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
void Before()
{
    PIN_WriteErrorMessage("this is an error message containing 'quotes', \"double quotes\", <less && greater than>", 1001,
                          PIN_ERR_NONFATAL, 0);

    PIN_WriteErrorMessage("this is a non-fatal user specified error message", 1002, PIN_ERR_NONFATAL, 3, "toolarg0", "toolarg1",
                          "toolarg2");

    PIN_WriteErrorMessage("this is a fatal user specified error message", 1003, PIN_ERR_FATAL, 3, "toolarg3", "toolarg4",
                          "toolarg5");
}

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

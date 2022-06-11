/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
 This is a negative test.  You cannot call PIN_ExecuteAt() while
 holding the client lock.  Do not try this at home.
*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>

/* ===================================================================== */

VOID Hello(CONTEXT* ctxt)
{
    PIN_LockClient();

    // This should produce a pin error.
    //
    PIN_ExecuteAt(ctxt);

    PIN_UnlockClient();
}

/* ===================================================================== */

VOID ImageLoad(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, "hello");
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);

        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(Hello), IARG_CONTEXT, IARG_END);

        RTN_Close(rtn);
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

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
#include <stdlib.h>
#include "tool_macros.h"
using std::cout;
using std::endl;
using std::flush;

/* ===================================================================== */
VOID Boo(UINT32 num0, UINT32 num1, UINT32 arg0, UINT32 arg1, UINT32 num2, UINT32 num3, UINT32 num4, UINT32 arg2, UINT32 arg3)
{
    cout << "Original Arguments = ( " << arg0 << ", " << arg1 << ", " << arg2 << ", " << arg3 << " )" << endl << flush;

    cout << "New Arguments = ( " << num0 << ", " << num1 << ", " << num2 << ", " << num3 << ", " << num4 << " )" << endl << flush;
}

VOID BazReplaced(UINT32 arg0) { cout << "Baz Original Arguments = ( " << arg0 << " )" << endl << flush; }

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    cout << IMG_Name(img) << endl;

    RTN rtn = RTN_FindByName(img, C_MANGLE("Bar"));
    if (RTN_Valid(rtn))
    {
        if (!RTN_IsSafeForProbedReplacement(rtn))
        {
            cout << "Cannot replace " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            exit(1);
        }

        cout << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;

        PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "Bar", PIN_PARG(int), PIN_PARG(int), PIN_PARG(int),
                                     PIN_PARG(int), PIN_PARG_END());

        RTN_ReplaceSignatureProbed(rtn, AFUNPTR(Boo), IARG_PROTOTYPE, proto, IARG_UINT32, 100, IARG_UINT32, 200,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_UINT32, 300,
                                   IARG_UINT32, 400, IARG_UINT32, 500, IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 3, IARG_END);
        PROTO_Free(proto);
    }

    rtn = RTN_FindByName(img, C_MANGLE("Baz"));
    if (RTN_Valid(rtn))
    {
        if (!RTN_IsSafeForProbedReplacement(rtn))
        {
            cout << "Cannot replace " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            exit(1);
        }

        cout << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;

        PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "Baz", PIN_PARG(int), PIN_PARG_END());

        // Due to some bug in the translator which was fixed we are checking this scenario:
        // Checking that Baz() first parameter is being passed correctly when transferring it as the first parameter
        // to BazReplaced(). The check is being done in the makefile.
        RTN_ReplaceSignatureProbed(rtn, AFUNPTR(BazReplaced), IARG_PROTOTYPE, proto, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

        PROTO_Free(proto);
    }
}

/* ===================================================================== */
int main(INT32 argc, CHAR* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

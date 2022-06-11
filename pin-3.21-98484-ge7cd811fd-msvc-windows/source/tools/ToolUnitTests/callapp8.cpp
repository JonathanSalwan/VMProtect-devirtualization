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
using std::cout;
using std::endl;

/* ===================================================================== */

int myBar(CONTEXT* ctxt, AFUNPTR pf_Bar, int one, int two, int stop)
{
    cout << " myBar: Jitting Bar8()" << endl;

    int res;

    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, pf_Bar, NULL, PIN_PARG(int), &res, PIN_PARG(int), one,
                                PIN_PARG(int), two, PIN_PARG(int), stop, PIN_PARG_END());

    cout << " myBar: Returned from Bar8(); res = " << res << endl;

    return res;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    PROTO protoBar =
        PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "Bar8", PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG_END());

    RTN rtn = RTN_FindByName(img, "Bar8");
    if (RTN_Valid(rtn))
    {
        cout << " Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;

        RTN_ReplaceSignature(rtn, AFUNPTR(myBar), IARG_PROTOTYPE, protoBar, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_UINT32, 1,
                             IARG_UINT32, 2, IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_END);
    }

    PROTO_Free(protoBar);
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

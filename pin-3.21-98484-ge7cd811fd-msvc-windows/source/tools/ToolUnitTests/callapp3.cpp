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
using std::cout;
using std::endl;

/* ===================================================================== */
static long (*pf_bar)(long, long);

/* ===================================================================== */
long Boo(CONTEXT* ctxt, AFUNPTR pf_Blue, long one, long two)
{
    cout << "Jitting Blue3() with return value" << endl;

    long res;

    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, pf_Blue, NULL, PIN_PARG(long), &res, PIN_PARG(long),
                                one, PIN_PARG(long), two, PIN_PARG_END());

    cout << "Returned from Blue3(); res = " << res << endl;

    return res;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        PROTO proto = PROTO_Allocate(PIN_PARG(long), CALLINGSTD_DEFAULT, "Bar3", PIN_PARG(long), PIN_PARG(long), PIN_PARG_END());

        VOID* pf_Blue;
        RTN rtn1 = RTN_FindByName(img, "Blue3");
        if (RTN_Valid(rtn1))
            pf_Blue = reinterpret_cast< VOID* >(RTN_Address(rtn1));
        else
        {
            cout << "Blue3 cannot be found." << endl;
            exit(1);
        }

        RTN rtn = RTN_FindByName(img, "Bar3");
        if (RTN_Valid(rtn))
        {
            cout << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;

            pf_bar = (long (*)(long, long))RTN_ReplaceSignature(rtn, AFUNPTR(Boo), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_PTR,
                                                                pf_Blue, IARG_ADDRINT, 1, IARG_ADDRINT, 2, IARG_END);
        }
        PROTO_Free(proto);
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

/*
 * Copyright (C) 2007-2021 Intel Corporation.
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
#include <limits.h>
#include <stdlib.h>
using std::cout;
using std::endl;

/* ===================================================================== */
static short (*pf_bar)(short, short, unsigned short, long, long, unsigned long, short, short, unsigned short, long, long,
                       unsigned long);

/* ===================================================================== */
int Boo(CONTEXT* ctxt, AFUNPTR pf_Green)
{
    short ret = 0;

    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, pf_Green, NULL, PIN_PARG(short), &ret, PIN_PARG(short),
                                SHRT_MIN, PIN_PARG(short), SHRT_MAX, PIN_PARG(unsigned short), USHRT_MAX, PIN_PARG(long),
                                LONG_MIN, PIN_PARG(long), LONG_MAX, PIN_PARG(unsigned long), ULONG_MAX, PIN_PARG(short), SHRT_MIN,
                                PIN_PARG(short), SHRT_MAX, PIN_PARG(unsigned short), USHRT_MAX, PIN_PARG(long), LONG_MIN,
                                PIN_PARG(long), LONG_MAX, PIN_PARG(unsigned long), ULONG_MAX, PIN_PARG_END());

    if (ret == 1)
        cout << " Correct return value " << endl;
    else
        cout << "ret = " << ret << endl;

    return ret;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        PROTO proto = PROTO_Allocate(PIN_PARG(short), CALLINGSTD_DEFAULT, "Bar13", PIN_PARG(short), PIN_PARG(short),
                                     PIN_PARG(unsigned short), PIN_PARG(long), PIN_PARG(long), PIN_PARG(unsigned long),
                                     PIN_PARG(short), PIN_PARG(short), PIN_PARG(unsigned short), PIN_PARG(long), PIN_PARG(long),
                                     PIN_PARG(unsigned long), PIN_PARG_END());

        VOID* pf_Green;
        RTN rtn1 = RTN_FindByName(img, "Green13");
        if (RTN_Valid(rtn1))
            pf_Green = reinterpret_cast< VOID* >(RTN_Address(rtn1));
        else
        {
            cout << "Green13 cannot be found." << endl;
            exit(1);
        }

        RTN rtn = RTN_FindByName(img, "Bar13");
        if (RTN_Valid(rtn))
        {
            cout << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;

            pf_bar = (short (*)(short, short, unsigned short, long, long, unsigned long, short, short, unsigned short, long, long,
                                unsigned long))RTN_ReplaceSignature(rtn, AFUNPTR(Boo), IARG_PROTOTYPE, proto, IARG_CONTEXT,
                                                                    IARG_PTR, pf_Green, IARG_END);
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

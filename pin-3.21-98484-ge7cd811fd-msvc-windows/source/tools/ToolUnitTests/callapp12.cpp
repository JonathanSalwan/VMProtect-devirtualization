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
static char (*pf_bar)(int, int, unsigned int, signed char, signed char, unsigned char, int, int, unsigned int, signed char,
                      signed char, unsigned char);

/* ===================================================================== */
int Boo(CONTEXT* ctxt, AFUNPTR pf_Green)
{
    char ret = 0;

    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, pf_Green, NULL, PIN_PARG(char), &ret, PIN_PARG(int),
                                INT_MIN, PIN_PARG(int), INT_MAX, PIN_PARG(unsigned int), UINT_MAX, PIN_PARG(signed char),
                                SCHAR_MIN, PIN_PARG(signed char), SCHAR_MAX, PIN_PARG(unsigned char), UCHAR_MAX, PIN_PARG(int),
                                INT_MIN, PIN_PARG(int), INT_MAX, PIN_PARG(unsigned int), UINT_MAX, PIN_PARG(signed char),
                                SCHAR_MIN, PIN_PARG(signed char), SCHAR_MAX, PIN_PARG(unsigned char), UCHAR_MAX, PIN_PARG_END());

    if (ret == '1')
        cout << "Correct return value" << endl;
    else
        cout << "ret = " << ret << endl;

    return ret;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        PROTO proto = PROTO_Allocate(PIN_PARG(char), CALLINGSTD_DEFAULT, "Bar12", PIN_PARG(int), PIN_PARG(int),
                                     PIN_PARG(unsigned int), PIN_PARG(signed char), PIN_PARG(signed char),
                                     PIN_PARG(unsigned char), PIN_PARG(int), PIN_PARG(int), PIN_PARG(unsigned int),
                                     PIN_PARG(signed char), PIN_PARG(signed char), PIN_PARG(unsigned char), PIN_PARG_END());

        VOID* pf_Green;
        RTN rtn1 = RTN_FindByName(img, "Green12");
        if (RTN_Valid(rtn1))
            pf_Green = reinterpret_cast< VOID* >(RTN_Address(rtn1));
        else
        {
            cout << "Green12 cannot be found." << endl;
            exit(1);
        }

        RTN rtn = RTN_FindByName(img, "Bar12");
        if (RTN_Valid(rtn))
        {
            cout << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;

            pf_bar =
                (char (*)(int, int, unsigned int, signed char, signed char, unsigned char, int, int, unsigned int, signed char,
                          signed char, unsigned char))RTN_ReplaceSignature(rtn, AFUNPTR(Boo), IARG_PROTOTYPE, proto, IARG_CONTEXT,
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

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
#include <stdlib.h>

/* ===================================================================== */
typedef int(__stdcall* FuncType)(int, int, int, int, int, int, int, int, int, int);

/* ===================================================================== */
int Boo(CONTEXT* ctxt, AFUNPTR pf_Blue, int one, int two, int three, int four, int five, int six, int seven, int eight, int nine,
        int zero)
{
    cout << "Inside Boo(" << one << ", " << two << ", " << three << ", " << four << ", " << five << ", " << six << ", " << seven
         << ", " << eight << ", " << nine << ", " << zero << ")" << endl;

    int ret = 0;

    cout << "Jitting Blue10() with ten arguments and one return value." << endl;

    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_STDCALL, pf_Blue, NULL, PIN_PARG(int), &ret, PIN_PARG(int), one,
                                PIN_PARG(int), two, PIN_PARG(int), three, PIN_PARG(int), four, PIN_PARG(int), five, PIN_PARG(int),
                                six, PIN_PARG(int), seven, PIN_PARG(int), eight, PIN_PARG(int), nine, PIN_PARG(int), zero,
                                PIN_PARG_END());

    cout << "Returned from Blue(); ret = " << ret << endl;

    return ret;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        PROTO proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_STDCALL, "StdBar10", PIN_PARG(int), PIN_PARG(int), PIN_PARG(int),
                                     PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int),
                                     PIN_PARG(int), PIN_PARG_END());

        VOID* pf_Blue;
        RTN rtn1 = RTN_FindByName(img, "StdBlue10");
        if (RTN_Valid(rtn1))
            pf_Blue = reinterpret_cast< VOID* >(RTN_Address(rtn1));
        else
        {
            cout << "StdBlue10 cannot be found." << endl;
            exit(1);
        }

        RTN rtn = RTN_FindByName(img, "StdBar10");
        if (RTN_Valid(rtn))
        {
            cout << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;

            RTN_ReplaceSignature(rtn, AFUNPTR(Boo), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_PTR, pf_Blue, IARG_UINT32, 1,
                                 IARG_UINT32, 2, IARG_UINT32, 3, IARG_UINT32, 4, IARG_UINT32, 5, IARG_UINT32, 6, IARG_UINT32, 7,
                                 IARG_UINT32, 8, IARG_UINT32, 9, IARG_UINT32, 0, IARG_END);
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

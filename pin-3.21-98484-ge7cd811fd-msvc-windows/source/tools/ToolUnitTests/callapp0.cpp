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
using std::dec;
using std::endl;
using std::hex;

/* ===================================================================== */
static void (*pf_bar)();

/* ===================================================================== */
VOID Boo(CONTEXT* ctxt, AFUNPTR pf_Blue)
{
    cout << "Jitting Blue() with no arguments; application address: " << hex << (unsigned long)pf_Blue << dec << endl;

    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, pf_Blue, NULL, PIN_PARG_END());

    cout << "Returned from Blue(); Native execution. " << endl;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "Bar", PIN_PARG_END());
        VOID* pf_Blue;

        RTN rtn1 = RTN_FindByName(img, "Blue");
        if (RTN_Valid(rtn1))
            pf_Blue = reinterpret_cast< VOID* >(RTN_Address(rtn1));
        else
        {
            cout << "Blue1 cannot be found." << endl;
            exit(1);
        }

        RTN rtn = RTN_FindByName(img, "Bar");
        if (RTN_Valid(rtn))
        {
            cout << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;

            pf_bar = (void (*)())RTN_ReplaceSignature(rtn, AFUNPTR(Boo), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_PTR, pf_Blue,
                                                      IARG_END);
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

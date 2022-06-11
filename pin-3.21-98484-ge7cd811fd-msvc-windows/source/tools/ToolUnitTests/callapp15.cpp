/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
  Insert a call to an analysis routine in probe mode.  From the analysis
  routine, call an application function using a function pointer.
*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>
#include <stdlib.h>
using std::cout;
using std::endl;

/* ===================================================================== */

int myBlue(CONTEXT* ctxt, AFUNPTR pf_Blue, int one, int two)
{
    cout << " myBlue: Jitting Blue6() at address " << hexstr(ADDRINT(pf_Blue)) << endl;

    int res;

    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, pf_Blue, NULL, PIN_PARG(int), &res, PIN_PARG(int), one,
                                PIN_PARG(int), two, PIN_PARG_END());

    cout << " myBlue: Returned from Blue6(); res = " << res << endl;

    return res;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        PROTO protoBlue =
            PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "Blue6", PIN_PARG(int), PIN_PARG(int), PIN_PARG_END());

        RTN blueRtn = RTN_FindByName(img, "Blue6");
        if (!RTN_Valid(blueRtn))
        {
            cout << "Blue6 cannot be found." << endl;
            exit(1);
        }

        RTN rtn = RTN_FindByName(img, "main");
        if (RTN_Valid(rtn))
        {
            RTN_Open(rtn);

            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                if (INS_IsCall(ins))
                {
                    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(myBlue), IARG_PROTOTYPE, protoBlue, IARG_CONTEXT, IARG_ADDRINT,
                                   AFUNPTR(RTN_Address(blueRtn)), IARG_UINT32, 1, IARG_UINT32, 2, IARG_END);

                    cout << " Instrumenting " << RTN_Name(rtn) << " at address " << hexstr(INS_Address(ins)) << endl;
                }
            }

            RTN_Close(rtn);
        }
        PROTO_Free(protoBlue);
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

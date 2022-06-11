/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool demonstrates how to get the value of the application's
// error code on windows in jit mode.
//

#include "pin.H"
#include <iostream>
#include <stdlib.h>
#include <errno.h>

using std::cerr;
using std::cout;
using std::endl;
namespace WINDOWS
{
#include <windows.h>
}

AFUNPTR pfnGetLastError = 0;

/* ===================================================================== */
VOID ToolCheckError(CONTEXT* ctxt)
{
    unsigned long err_code;

    if (pfnGetLastError != 0)
    {
        cerr << "Tool: calling GetLastError()" << endl;

        PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, pfnGetLastError, NULL, PIN_PARG(unsigned long),
                                    &err_code, PIN_PARG_END());

        cerr << "Tool: error code=" << err_code << endl;
    }
    else
        cerr << "Tool: GetLastError() not found." << endl;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "CheckError", PIN_PARG_END());

        RTN rtn = RTN_FindByName(img, "CheckError");
        if (RTN_Valid(rtn))
        {
            cout << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;

            RTN_ReplaceSignature(rtn, AFUNPTR(ToolCheckError), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_END);
        }
        PROTO_Free(proto);
    }
}

/* ===================================================================== */
int main(INT32 argc, CHAR* argv[])
{
    pfnGetLastError = (AFUNPTR)WINDOWS::GetProcAddress(WINDOWS::GetModuleHandle("kernel32.dll"), "GetLastError");

    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool demonstrates how to get the value of the application's
// errno on linux in jit mode.
//
// There are symbols called __errno_location in both libc and libpthread.
// This test captures the address of the __errno_location() function in
// one of the libraries and not in the other.   Today, the implementations
// in libc and libpthread point to the same actual location (in a TLS),
// but this assumption is fragile and may change at some time int he future.

#include "pin.H"
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include "tool_macros.h"
using std::cerr;
using std::cout;
using std::endl;
using std::hex;

#if defined(TARGET_MAC)
#define ERRNO_SYMBOL ("___error")
#else
#define ERRNO_SYMBOL ("__errno_location")
#endif

AFUNPTR pf_errno_location = 0;

/* ===================================================================== */
VOID ToolCheckError(CONTEXT* ctxt)
{
    unsigned long err_loc;

    if (*pf_errno_location != 0)
    {
        cerr << "Tool: calling __errno_location()" << endl;

        PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, pf_errno_location, NULL, PIN_PARG(unsigned long),
                                    &err_loc, PIN_PARG_END());

        int err_value = *(reinterpret_cast< unsigned long* >(err_loc));

        cerr << "Tool: errno=" << err_value << endl;
    }
    else
        cerr << "Tool: __errno_location not found." << endl;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    RTN errno_location_rtn = RTN_FindByName(img, ERRNO_SYMBOL);
    if (RTN_Valid(errno_location_rtn))
    {
        pf_errno_location = reinterpret_cast< AFUNPTR >(RTN_Address(errno_location_rtn));
        cerr << "Tool: Found __errno_location() at " << hex << (ADDRINT)pf_errno_location << "." << endl;
    }

    if (IMG_IsMainExecutable(img))
    {
        PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "CheckError", PIN_PARG_END());

        RTN rtn = RTN_FindByName(img, C_MANGLE("CheckError"));
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
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

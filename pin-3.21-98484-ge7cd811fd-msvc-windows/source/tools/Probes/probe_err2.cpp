/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
 *  This file tests error reporting.
*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>
#include "tool_macros.h"

/* ===================================================================== */

/* ===================================================================== */
void* Malloc(CONTEXT* ctxt, AFUNPTR pf_malloc, size_t size)
{
    void* res;

    // PIN_CallApplicationFunction() cannot be called in Probe mode!
    // This should result in an error.  Do not try this at home.
    //
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, pf_malloc, NULL, PIN_PARG(int), &res, PIN_PARG(size_t),
                                size, PIN_PARG_END());

    return res;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    PROTO proto = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "malloc", PIN_PARG(size_t), PIN_PARG_END());

    RTN rtn = RTN_FindByName(img, C_MANGLE("malloc"));
    if (RTN_Valid(rtn))
    {
        RTN_ReplaceSignatureProbed(rtn, AFUNPTR(Malloc), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
    }
    PROTO_Free(proto);
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

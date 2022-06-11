/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
 * Insert a call before a function in probe mode.
 */

/* ===================================================================== */
#include "pin.H"
#include <iostream>
#include <stdlib.h>
#include "tool_macros.h"
using std::cout;
using std::endl;
using std::flush;

/* ===================================================================== */
/* Analysis routines  */
/* ===================================================================== */

VOID Before(size_t size) { cout << "Calling my_malloc() with size=" << size << endl << flush; }

/* ===================================================================== */
/* Instrumentation routines  */
/* ===================================================================== */

VOID Sanity(IMG img, RTN rtn)
{
    if (PIN_IsProbeMode() && !RTN_IsSafeForProbedInsertion(rtn))
    {
        cout << "Cannot insert calls around " << RTN_Name(rtn) << "() in " << IMG_Name(img) << endl;
        exit(1);
    }
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img)) return;

    PROTO proto_malloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "my_malloc", PIN_PARG(size_t), PIN_PARG_END());

    RTN rtn = RTN_FindByName(img, C_MANGLE("my_malloc"));
    if (RTN_Valid(rtn))
    {
        Sanity(img, rtn);

        cout << "Inserting call before my_malloc in " << IMG_Name(img) << endl;

        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before), IARG_PROTOTYPE, proto_malloc, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                             IARG_END);
    }

    PROTO_Free(proto_malloc);
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

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

VOID Before1(size_t size) { cout << "Before 1: Calling my_malloc() with size=" << size << endl << flush; }

VOID Before2(size_t size) { cout << "Before 2: Calling my_malloc() with size=" << size << endl << flush; }

VOID Before3(size_t size) { cout << "Before 3: Calling my_malloc() with size=" << size << endl << flush; }

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

    RTN rtn = RTN_FindByName(img, C_MANGLE("my_malloc"));
    if (RTN_Valid(rtn))
    {
        Sanity(img, rtn);

        cout << "Inserting 3 calls before my_malloc in " << IMG_Name(img) << endl;

        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before1), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before2), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before3), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
    }
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

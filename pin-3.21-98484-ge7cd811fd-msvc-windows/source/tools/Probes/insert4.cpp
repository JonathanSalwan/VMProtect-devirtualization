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
using std::dec;
using std::endl;
using std::flush;
using std::hex;

/* ===================================================================== */
/* Analysis routines  */
/* ===================================================================== */

VOID Before_Malloc0(size_t size) { cout << "Before_Malloc0: Calling malloc() with size=" << size << endl << flush; }

VOID Before_Malloc1(size_t size) { cout << "Before_Malloc1: Calling malloc() with size=" << size << endl << flush; }

VOID Before_Free0(void* ptr) { cout << "Before_Free0: Calling free() with ptr=" << hex << (ADDRINT)ptr << dec << endl << flush; }

VOID Before_Free1(void* ptr) { cout << "Before_Free1: Calling free() with ptr=" << hex << (ADDRINT)ptr << dec << endl << flush; }

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
    RTN rtn = RTN_FindByName(img, C_MANGLE("malloc"));
    if (RTN_Valid(rtn))
    {
        Sanity(img, rtn);

        cout << "Inserting 2 calls before malloc in " << IMG_Name(img) << endl;

        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before_Malloc0), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before_Malloc1), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
    }

    rtn = RTN_FindByName(img, C_MANGLE("free"));
    if (RTN_Valid(rtn))
    {
        Sanity(img, rtn);

        cout << "Inserting 2 calls before free in " << IMG_Name(img) << endl;

        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before_Free0), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before_Free1), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
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

/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
 * Insert a call before/after a function in probe mode.
 */

/* ===================================================================== */
#include "pin.H"
#include <cstdlib>
#include <iostream>
#include "tool_macros.h"
using std::cout;
using std::endl;
using std::flush;

/* ===================================================================== */
/* Analysis routines  */
/* ===================================================================== */

VOID Before(UINT32 arg0, UINT32 arg1, UINT32 arg2, UINT32 arg3, UINT32 arg4, UINT32 arg5, UINT32 arg6, UINT32 arg7, UINT32 arg8,
            UINT32 arg9)
{
    cout << "Before: original arguments = ( " << arg0 << ", " << arg1 << ", " << arg2 << ", " << arg3 << ", " << arg4 << ", "
         << arg5 << ", " << arg6 << ", " << arg7 << ", " << arg8 << ", " << arg9 << " )" << endl
         << flush;
}

VOID After(ADDRINT retval) { cout << "After: return value = " << retval << endl << flush; }

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
    RTN rtn = RTN_FindByName(img, C_MANGLE("Bar10"));
    if (RTN_Valid(rtn))
    {
        Sanity(img, rtn);

        cout << "Inserting calls before/after Bar10 in " << IMG_Name(img) << endl;

        PROTO proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "Bar10", PIN_PARG(int), PIN_PARG(int), PIN_PARG(int),
                                     PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int),
                                     PIN_PARG(int), PIN_PARG_END());

        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before), IARG_PROTOTYPE, proto, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 4, IARG_FUNCARG_ENTRYPOINT_VALUE, 5, IARG_FUNCARG_ENTRYPOINT_VALUE, 6,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 7, IARG_FUNCARG_ENTRYPOINT_VALUE, 8, IARG_FUNCARG_ENTRYPOINT_VALUE, 9,
                             IARG_END);

        RTN_InsertCallProbed(rtn, IPOINT_AFTER, AFUNPTR(After), IARG_PROTOTYPE, proto, IARG_REG_VALUE, REG_GAX, IARG_END);

        PROTO_Free(proto);
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

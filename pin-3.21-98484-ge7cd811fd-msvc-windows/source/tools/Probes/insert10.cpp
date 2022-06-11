/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
 * Insert a call after a function in probe mode.
 */

/* ===================================================================== */
#include "pin.H"

#if defined(TARGET_WINDOWS)
namespace WINDOWS
{
#include <Windows.h>
}
#endif
#include <cstdlib>
#include <iostream>
#include "tool_macros.h"
using std::cout;
using std::endl;
using std::flush;

/* ===================================================================== */
/* Analysis routines  */
/* ===================================================================== */

VOID After0(INT32 arg0, INT32 arg1, INT32 arg2, INT32 arg3, INT32 arg4, INT32 arg5, INT32 arg6, INT32 arg7, INT32 arg8,
            INT32 arg9)
{
    cout << "After0: arguments = ( " << arg0 << ", " << arg1 << ", " << arg2 << ", " << arg3 << ", " << arg4 << ", " << arg5
         << ", " << arg6 << ", " << arg7 << ", " << arg8 << ", " << arg9 << " )" << endl
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

        cout << "Inserting calls after Bar10 in " << IMG_Name(img) << endl;

        PROTO proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "Bar10", PIN_PARG(int), PIN_PARG(int), PIN_PARG(int),
                                     PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int),
                                     PIN_PARG(int), PIN_PARG_END());

        RTN_InsertCallProbed(rtn, IPOINT_AFTER, AFUNPTR(After), IARG_PROTOTYPE, proto, IARG_REG_VALUE, REG_GAX, IARG_END);

        RTN_InsertCallProbed(rtn, IPOINT_AFTER, AFUNPTR(After0), IARG_PROTOTYPE, proto, IARG_UINT32, 1, IARG_UINT32, 2,
                             IARG_UINT32, 3, IARG_UINT32, 4, IARG_UINT32, 5, IARG_UINT32, 6, IARG_UINT32, 7, IARG_UINT32, 8,
                             IARG_UINT32, 9, IARG_UINT32, 0, IARG_END);

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

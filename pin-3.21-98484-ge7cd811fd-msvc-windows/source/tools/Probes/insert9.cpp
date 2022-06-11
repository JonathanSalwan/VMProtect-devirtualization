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
#include <cstdlib>
#include <iostream>
#include "tool_macros.h"
using std::cout;
using std::dec;
using std::endl;
using std::flush;
using std::hex;

/* ===================================================================== */
/* Analysis routines  */
/* ===================================================================== */

VOID After_Malloc(ADDRINT retval)
{
    cout << "After_Malloc: my_malloc() return value = " << hex << retval << dec << endl << flush;
}

VOID After_Free() { cout << "After_Free: returning from my_free()." << endl << flush; }

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
    RTN rtn = RTN_FindByName(img, C_MANGLE("my_malloc"));
    if (RTN_Valid(rtn))
    {
        Sanity(img, rtn);

        cout << "Inserting calls after my_malloc in " << IMG_Name(img) << endl;

        PROTO proto_malloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "my_malloc", PIN_PARG(size_t), PIN_PARG_END());

        RTN_InsertCallProbed(rtn, IPOINT_AFTER, AFUNPTR(After_Malloc), IARG_PROTOTYPE, proto_malloc, IARG_REG_VALUE, REG_GAX,
                             IARG_END);

        PROTO_Free(proto_malloc);
    }

    rtn = RTN_FindByName(img, C_MANGLE("my_free"));
    if (RTN_Valid(rtn))
    {
        Sanity(img, rtn);

        cout << "Inserting calls after my_free in " << IMG_Name(img) << endl;

        PROTO proto_free = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "my_free", PIN_PARG(void*), PIN_PARG_END());

        RTN_InsertCallProbed(rtn, IPOINT_AFTER, AFUNPTR(After_Free), IARG_PROTOTYPE, proto_free, IARG_END);

        PROTO_Free(proto_free);
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

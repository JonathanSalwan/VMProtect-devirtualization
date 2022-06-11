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
using std::dec;
using std::endl;
using std::flush;
using std::hex;

/* ===================================================================== */
/* Analysis routines  */
/* ===================================================================== */

VOID Before(ADDRINT ebx, ADDRINT ebp, ADDRINT esp)
{
    cout << "Before:   ebx/r15 = " << hex << ebx << "  ebp/rbp = " << ebp << "  esp/rsp = " << esp << dec << endl;
}

VOID After(REG reg) { cout << "After: return value = " << reg << endl << flush; }

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
    RTN rtn = RTN_FindByName(img, C_MANGLE("Bar9"));
    if (RTN_Valid(rtn))
    {
        Sanity(img, rtn);

        PROTO proto =
            PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "Bar9", PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int),
                           PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG_END());
#if defined(TARGET_IA32)
        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before), IARG_PROTOTYPE, proto, IARG_REG_VALUE, REG_EBX, IARG_REG_VALUE,
                             REG_EBP, IARG_REG_VALUE, REG_ESP, IARG_END);
#else
        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before), IARG_PROTOTYPE, proto, IARG_REG_VALUE, REG_R15, IARG_REG_VALUE,
                             REG_RBP, IARG_REG_VALUE, REG_RSP, IARG_END);
#endif
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

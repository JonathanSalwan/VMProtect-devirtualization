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
#include <iostream>

using std::cout;
using std::endl;
using std::flush;
using std::string;

/* ===================================================================== */
typedef int(__stdcall* FuncType)(int, int, int, int, int, int, int, int, int, int);

/* ===================================================================== */
/* Analysis routines  */
/* ===================================================================== */

VOID Before(INT32 arg0, INT32 arg1, INT32 arg2, INT32 arg3, INT32 arg4, INT32 arg5, INT32 arg6, INT32 arg7, INT32 arg8,
            INT32 arg9)
{
    cout << "Before: arguments = ( " << arg0 << ", " << arg1 << ", " << arg2 << ", " << arg3 << ", " << arg4 << ", " << arg5
         << ", " << arg6 << ", " << arg7 << ", " << arg8 << ", " << arg9 << " )" << endl
         << flush;
}

VOID After0(INT32 arg0, INT32 arg1, INT32 arg2, INT32 arg3, INT32 arg4, INT32 arg5, INT32 arg6, INT32 arg7, INT32 arg8,
            INT32 arg9)
{
    cout << "After0: arguments = ( " << arg0 << ", " << arg1 << ", " << arg2 << ", " << arg3 << ", " << arg4 << ", " << arg5
         << ", " << arg6 << ", " << arg7 << ", " << arg8 << ", " << arg9 << " )" << endl
         << flush;
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
    if (IMG_IsMainExecutable(img))
    {
        // Walk through the symbols in the symbol table.
        //
        for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym))
        {
            string undFuncName = PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY);

            //  Find the StdBar10() function.
            if (undFuncName == "StdBar10")
            {
                RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
                if (RTN_Valid(rtn))
                {
                    Sanity(img, rtn);

                    cout << "Inserting calls before/after StdBar10 in " << IMG_Name(img) << endl;

                    PROTO proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_STDCALL, "StdBar10", PIN_PARG(int), PIN_PARG(int),
                                                 PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int),
                                                 PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG_END());

                    RTN_InsertCallProbed(rtn, IPOINT_AFTER, AFUNPTR(After), IARG_PROTOTYPE, proto, IARG_REG_VALUE, REG_GAX,
                                         IARG_END);

                    RTN_InsertCallProbed(rtn, IPOINT_AFTER, AFUNPTR(After0), IARG_PROTOTYPE, proto, IARG_UINT32, 1, IARG_UINT32,
                                         2, IARG_UINT32, 3, IARG_UINT32, 4, IARG_UINT32, 5, IARG_UINT32, 6, IARG_UINT32, 7,
                                         IARG_UINT32, 8, IARG_UINT32, 9, IARG_UINT32, 0, IARG_END);

                    RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before), IARG_PROTOTYPE, proto,
                                         IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                                         IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
                                         IARG_FUNCARG_ENTRYPOINT_VALUE, 4, IARG_FUNCARG_ENTRYPOINT_VALUE, 5,
                                         IARG_FUNCARG_ENTRYPOINT_VALUE, 6, IARG_FUNCARG_ENTRYPOINT_VALUE, 7,
                                         IARG_FUNCARG_ENTRYPOINT_VALUE, 8, IARG_FUNCARG_ENTRYPOINT_VALUE, 9, IARG_END);

                    PROTO_Free(proto);
                }
                else
                    cout << "Cannot find StdBar10" << endl;
            }
        }
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

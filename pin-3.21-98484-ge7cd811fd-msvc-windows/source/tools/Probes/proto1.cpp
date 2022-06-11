/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
  Replace an original function with a custom function defined in the tool. The
  new function can have either the same or different signature from that of its
  original function.
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
/* Arrays needed for multiple replacements. */
/* ===================================================================== */

typedef VOID* (*FUNCPTR_MALLOC)(size_t);

/* ===================================================================== */
/* ===================================================================== */
/* Replacement routine support  */
/* ===================================================================== */

VOID* Malloc_IA32(FUNCPTR_MALLOC orgFuncptr, UINT32 arg0, ADDRINT returnIp, ADDRINT esp, ADDRINT ebp, ADDRINT ebx)
{
    cout << "Malloc_IA32 "
         << "(" << hex << (ADDRINT)orgFuncptr << ", " << hex << arg0 << ", " << hex << returnIp << ", " << hex << ebx << ", "
         << hex << esp << ", " << hex << ebp << ")" << endl
         << flush;

    VOID* v = orgFuncptr(arg0);

    cout << "Return value = " << hex << (ADDRINT)v << dec << endl << flush;

    return v;
}

/* ===================================================================== */
/* ===================================================================== */
/* Instrumentation routine support  */
/* ===================================================================== */

VOID ImageLoad(IMG img, VOID* v)
{
    cout << IMG_Name(img) << endl;

#if defined(TARGET_IA32)
    PROTO proto_malloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_CDECL, "malloc", PIN_PARG(int), PIN_PARG_END());

#elif defined(TARGET_IA32E)
    PROTO proto_malloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "malloc", PIN_PARG(int), PIN_PARG_END());
#endif

    RTN rtn = RTN_FindByName(img, C_MANGLE("malloc"));
    if (RTN_Valid(rtn))
    {
        if (!RTN_IsSafeForProbedReplacement(rtn))
        {
            cout << "Cannot replace malloc in " << IMG_Name(img) << endl;
            exit(1);
        }

        cout << "Replacing malloc in " << IMG_Name(img) << endl;

#if defined(TARGET_IA32)
        RTN_ReplaceSignatureProbed(rtn, AFUNPTR(Malloc_IA32), IARG_PROTOTYPE, proto_malloc, IARG_ORIG_FUNCPTR,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_RETURN_IP, IARG_REG_VALUE, REG_ESP, IARG_REG_VALUE,
                                   REG_EBP, IARG_REG_VALUE, REG_EBX, IARG_END);
#else
        RTN_ReplaceSignatureProbed(rtn, AFUNPTR(Malloc_IA32), IARG_PROTOTYPE, proto_malloc, IARG_ORIG_FUNCPTR,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_RETURN_IP, IARG_REG_VALUE, REG_RSP, IARG_REG_VALUE,
                                   REG_RBP, IARG_REG_VALUE, REG_R15, IARG_END);
#endif
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

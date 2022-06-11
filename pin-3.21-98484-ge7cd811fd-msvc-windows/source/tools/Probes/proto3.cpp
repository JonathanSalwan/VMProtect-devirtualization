/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
  Replace an original function with a custom function defined in the tool. The
  new function can have either the same or different signature from that of its
  original function.  This tool works in both JIT mode and Probe mode.
*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>
#include <stdlib.h>
#include "tool_macros.h"
using std::cout;
using std::dec;
using std::endl;
using std::hex;

/* ===================================================================== */
/* Arrays needed for multiple replacements. */
/* ===================================================================== */

typedef VOID* (*FUNCPTR_MALLOC)(size_t);

VOID Sanity(IMG img, RTN rtn);
VOID ReplaceProbed(RTN rtn, PROTO proto_malloc);
VOID ReplaceJitted(RTN rtn, PROTO proto_malloc);
VOID* Probe_Malloc_IA32(FUNCPTR_MALLOC orgFuncptr, size_t arg0);
VOID* Jit_Malloc_IA32(CONTEXT* context, AFUNPTR orgFuncptr, size_t arg0);

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    cout << IMG_Name(img) << endl;

    PROTO proto_malloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "malloc", PIN_PARG(size_t), PIN_PARG_END());

    RTN rtn = RTN_FindByName(img, C_MANGLE("malloc"));
    if (RTN_Valid(rtn))
    {
        Sanity(img, rtn);

        cout << "Replacing malloc in " << IMG_Name(img) << endl;

        if (PIN_IsProbeMode())
            ReplaceProbed(rtn, proto_malloc);
        else
            ReplaceJitted(rtn, proto_malloc);
    }

    PROTO_Free(proto_malloc);
}

/* ===================================================================== */

VOID Sanity(IMG img, RTN rtn)
{
    if (PIN_IsProbeMode() && !RTN_IsSafeForProbedReplacement(rtn))
    {
        cout << "Cannot replace malloc in " << IMG_Name(img) << endl;
        exit(1);
    }
}

/* ===================================================================== */

VOID ReplaceProbed(RTN rtn, PROTO proto_malloc)
{
    RTN_ReplaceSignatureProbed(rtn, AFUNPTR(Probe_Malloc_IA32), IARG_PROTOTYPE, proto_malloc, IARG_ORIG_FUNCPTR,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
}

/* ===================================================================== */

VOID ReplaceJitted(RTN rtn, PROTO proto_malloc)
{
    RTN_ReplaceSignature(rtn, AFUNPTR(Jit_Malloc_IA32), IARG_PROTOTYPE, proto_malloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                         IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
}

/* ===================================================================== */

VOID* Probe_Malloc_IA32(FUNCPTR_MALLOC orgFuncptr, size_t arg0)
{
    cout << "Probe_Malloc_IA32(" << hex << (ADDRINT)orgFuncptr << ", " << hex << arg0 << ") " << dec << endl;

    VOID* v = orgFuncptr(arg0);
    return v;
}

/* ===================================================================== */

VOID* Jit_Malloc_IA32(CONTEXT* context, AFUNPTR orgFuncptr, size_t arg0)
{
    cout << "Jit_Malloc_IA32(" << hex << (ADDRINT)context << ", " << hex << (ADDRINT)orgFuncptr << ", " << hex << arg0 << ") "
         << dec << endl;

    VOID* ret;

    PIN_CallApplicationFunction(context, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFuncptr, NULL, PIN_PARG(void*), &ret,
                                PIN_PARG(size_t), arg0, PIN_PARG_END());

    cout << "Jit_Malloc_IA32: ret = " << hex << (ADDRINT)ret << dec << endl;
    return ret;
}

/* ===================================================================== */

int main(INT32 argc, CHAR* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    if (PIN_IsProbeMode())
        PIN_StartProgramProbed();
    else
        PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

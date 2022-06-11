/*
 * Copyright (C) 2007-2021 Intel Corporation.
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
using std::endl;

/* ===================================================================== */
/* Arrays needed for multiple replacements. */
/* ===================================================================== */

typedef VOID* (*FUNCPTR_MALLOC)(size_t);

VOID Sanity(IMG img, RTN rtn);
VOID ReplaceProbed(RTN rtn, PROTO proto_malloc);
VOID ReplaceJitted(RTN rtn, PROTO proto_malloc);
VOID* Probe_Malloc_IA32(FUNCPTR_MALLOC orgFuncptr, size_t arg0);
VOID* Jit_Malloc_IA32(CONTEXT* context, AFUNPTR orgFuncptr, size_t arg0);
VOID PrintContext(CONTEXT* ctxt);

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img)) return;

    cout << IMG_Name(img) << endl;

    PROTO proto_malloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "my_malloc", PIN_PARG(size_t), PIN_PARG_END());

    RTN rtn = RTN_FindByName(img, C_MANGLE("my_malloc"));
    if (RTN_Valid(rtn))
    {
        Sanity(img, rtn);

        cout << "Replacing my_malloc in " << IMG_Name(img) << endl;

        if (PIN_IsProbeMode())
            ReplaceProbed(rtn, proto_malloc);
        else
            ReplaceJitted(rtn, proto_malloc);
    }

    PROTO_Free(proto_malloc);
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
/* Replacement routine support  */
/* ===================================================================== */

VOID Sanity(IMG img, RTN rtn)
{
    if (PIN_IsProbeMode() && !RTN_IsSafeForProbedReplacement(rtn))
    {
        cout << "Cannot replace " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
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
    VOID* v = orgFuncptr(arg0);
    return v;
}

/* ===================================================================== */

VOID* Jit_Malloc_IA32(CONTEXT* context, AFUNPTR orgFuncptr, size_t arg0)
{
    VOID* ret;

    PIN_CallApplicationFunction(context, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFuncptr, NULL, PIN_PARG(void*), &ret,
                                PIN_PARG(size_t), arg0, PIN_PARG_END());

    return ret;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

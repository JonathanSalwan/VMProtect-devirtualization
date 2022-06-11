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

namespace WIND
{
#include <windows.h>
}

using std::cout;
using std::dec;
using std::endl;
using std::flush;
using std::hex;
using std::string;

typedef VOID* (*FUNCPTR_MALLOC)(size_t);

/* ===================================================================== */
VOID* Jit_Malloc_IA32(CONTEXT* context, FUNCPTR_MALLOC orgFuncptr, size_t arg0, ADDRINT returnIp, ADDRINT esp, ADDRINT ebp)
{
    cout << "Jit_Malloc_IA32 (" << hex << (ADDRINT)orgFuncptr << ", " << hex << arg0 << ", " << hex << returnIp << "," << hex
         << esp << ", " << hex << ebp << ")" << endl
         << flush;

    VOID* ret;

    PIN_CallApplicationFunction(context, PIN_ThreadId(), CALLINGSTD_DEFAULT, AFUNPTR(orgFuncptr), NULL, PIN_PARG(void*), &ret,
                                PIN_PARG(size_t), arg0, PIN_PARG_END());

    cout << "Jit_Malloc_IA32: ret = " << hex << (ADDRINT)ret << dec << endl << flush;
    return ret;
}

/* ===================================================================== */
VOID* Probe_Malloc_IA32(FUNCPTR_MALLOC orgFuncptr, size_t arg0, ADDRINT returnIp, ADDRINT esp, ADDRINT ebp)
{
    cout << "Probe_Malloc_IA32 (" << hex << (ADDRINT)orgFuncptr << ", " << hex << arg0 << ", " << hex << returnIp << "," << hex
         << esp << ", " << hex << ebp << ")" << endl
         << flush;

    VOID* ret;

    ret = orgFuncptr(arg0);

    cout << "Probe_Malloc_IA32: ret = " << hex << (ADDRINT)ret << dec << endl << flush;
    return ret;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    const char* name = "malloc";

    RTN rtn = RTN_FindByName(img, name);
    if (RTN_Valid(rtn))
    {
        PROTO proto_malloc = PROTO_Allocate(PIN_PARG(WIND::LPVOID), CALLINGSTD_DEFAULT, name, PIN_PARG(size_t), PIN_PARG_END());

        cout << "Replacing " << name << " in " << IMG_Name(img) << endl << flush;

        if (!PIN_IsProbeMode())
        {
            RTN_ReplaceSignature(rtn, AFUNPTR(Jit_Malloc_IA32), IARG_PROTOTYPE, proto_malloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                                 IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_RETURN_IP, IARG_REG_VALUE, REG_ESP, IARG_REG_VALUE,
                                 REG_EBP, IARG_END);
        }
        else if (RTN_IsSafeForProbedReplacement(rtn))
        {
            RTN_ReplaceSignatureProbed(rtn, AFUNPTR(Probe_Malloc_IA32), IARG_PROTOTYPE, proto_malloc, IARG_ORIG_FUNCPTR,
                                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_RETURN_IP, IARG_REG_VALUE, REG_ESP, IARG_REG_VALUE,
                                       REG_EBP, IARG_END);
        }
        else
        { // This is workaround for mantis 4588. When mantis is handled this code need to be addressed.
            cout << "Replacement not safe" << endl << flush;
        }

        PROTO_Free(proto_malloc);
    }
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

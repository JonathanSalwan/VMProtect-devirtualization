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

using std::cout;
using std::endl;
using std::flush;
using std::hex;
using std::string;

namespace WIND
{
#include <windows.h>
}

typedef VOID* (*FUNCPTR_MALLOC)(size_t);

/* ===================================================================== */
VOID* Probe_Malloc_IA32(FUNCPTR_MALLOC orgFuncptr, size_t arg0, ADDRINT returnIp, ADDRINT esp, ADDRINT ebp)
{
    cout << "Probe_Malloc_IA32 (" << hex << (ADDRINT)orgFuncptr << ", " << hex << arg0 << ", " << hex << returnIp << "," << hex
         << esp << ", " << hex << ebp << ")" << endl
         << flush;

    ASSERTX(esp <= 0xffffffff);

    VOID* v = orgFuncptr(arg0);
    return v;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    const char* name = "malloc";

    RTN rtn = RTN_FindByName(img, name);
    if (RTN_Valid(rtn) && RTN_IsSafeForProbedReplacement(rtn))
    {
        PROTO proto_funcptr = PROTO_Allocate(PIN_PARG(WIND::LPVOID), CALLINGSTD_DEFAULT, name, PIN_PARG(size_t), PIN_PARG_END());

        cout << "Replacing " << name << " in " << IMG_Name(img) << endl << flush;

        RTN_ReplaceSignatureProbed(rtn, AFUNPTR(Probe_Malloc_IA32), IARG_PROTOTYPE, proto_funcptr, IARG_ORIG_FUNCPTR,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_RETURN_IP, IARG_REG_VALUE, REG_ESP, IARG_REG_VALUE,
                                   REG_EBP, IARG_END);

        PROTO_Free(proto_funcptr);
    }
    else if (RTN_Valid(rtn))
    { // This is workaround for mantis 4588. When mantis is handled this code need to be addressed.
        cout << "Replacement not safe" << endl << flush;
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

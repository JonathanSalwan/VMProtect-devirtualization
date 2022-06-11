/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
  Replace an original function with a custom function defined in the tool. The
  new function can have either the same or different signature from that of its
  original function.
*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>
#include "tool_macros.h"
using std::cout;
using std::dec;
using std::endl;
using std::hex;

/* ===================================================================== */
static void (*pf_bar)();

/* ===================================================================== */
VOID Boo(CONTEXT* ctxt, AFUNPTR pf_Blue)
{
    ADDRINT sp = PIN_GetContextReg(ctxt, REG_STACK_PTR);
    ADDRINT pc = PIN_GetContextReg(ctxt, REG_INST_PTR);

    cout << "Context SP = " << hex << sp << "  Context PC = " << pc << dec << endl;

    pf_Blue();
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    cout << IMG_Name(img) << endl;

    PROTO proto   = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "Bar", PIN_PARG_END());
    VOID* pf_Blue = 0x0;

    RTN rtn1 = RTN_FindByName(img, C_MANGLE("Blue"));
    if (RTN_Valid(rtn1)) pf_Blue = reinterpret_cast< VOID* >(RTN_Address(rtn1));

    RTN rtn = RTN_FindByName(img, C_MANGLE("Bar"));
    if (RTN_Valid(rtn) && pf_Blue != 0x0)
    {
        cout << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;

        pf_bar = (void (*)())RTN_ReplaceSignatureProbed(rtn, AFUNPTR(Boo), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_PTR, pf_Blue,
                                                        IARG_END);
    }
    PROTO_Free(proto);
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

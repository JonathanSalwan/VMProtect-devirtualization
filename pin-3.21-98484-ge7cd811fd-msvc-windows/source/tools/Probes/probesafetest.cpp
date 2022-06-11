/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
   Test sefe probe setting.
*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
using std::string;

/* ===================================================================== */
/* Globals */
/* ===================================================================== */

typedef VOID (*FUNCPTR)(ADDRINT arg);

/* ===================================================================== */
/* Replacement routines  */
/* ===================================================================== */

VOID Replace_too_short_with_nops(FUNCPTR fp, CONTEXT* ctxt)
{
    printf("too_short_with_nops successfully replaced!!!\n");
    fflush(0);
}
VOID Replace_too_short(FUNCPTR fp, CONTEXT* ctxt)
{
    printf("too_short successfully replaced!!!\n");
    fflush(0);
}

/* ===================================================================== */
/* Instrumentation Routines  */
/* ===================================================================== */

void Replace(IMG img, string name, AFUNPTR funptr)
{
    RTN rtn = RTN_FindByName(img, name.c_str());

    if (RTN_Valid(rtn))
    {
        if (RTN_IsSafeForProbedReplacement(rtn))
        {
            printf("%s is safe for probed replacement !!!\n", name.c_str());
        }
        else
        {
            printf("%s is not safe for probed replacement !!!\n", name.c_str());
        }

        // force the replacement even when probe is not allowed
        PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, name.c_str(), PIN_PARG_END());

        RTN_ReplaceSignatureProbed(rtn, funptr, IARG_PROTOTYPE, proto, IARG_ORIG_FUNCPTR, IARG_CONTEXT, IARG_END);

        PROTO_Free(proto);
    }
}

VOID ImageLoad(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img)) return;

    Replace(img, "too_short", AFUNPTR(Replace_too_short));
    Replace(img, "too_short_with_nops", AFUNPTR(Replace_too_short_with_nops));
    fflush(0);
}

/* ===================================================================== */
/* Main  */
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

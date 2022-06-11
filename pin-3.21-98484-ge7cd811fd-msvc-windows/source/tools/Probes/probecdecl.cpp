/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
  Tests inserting a probe when the replacement function has the cdecl calling standard
*/

#include "pin.H"
#include <stdio.h>

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

FILE* fp;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

/* ===================================================================== */

//--------------------------------------------------------------------------------------------------
typedef void*(__cdecl* mallocType)(size_t size);
void* MallocProbe(mallocType mallocWithoutReplacement, size_t size, CONTEXT* pPinContext, ADDRINT returnIp)
{
    fprintf(fp, "Probe\n");
    fflush(fp);
    void* ptr = mallocWithoutReplacement(size);
    return ptr;
}

/* ===================================================================== */
// Called every time a new image is loaded
// Look for routines that we want to probe

static bool s_init = false;
static PROTO s_protoAlloc;
VOID ImageLoad(IMG img, VOID* v)
{
    if (!s_init)
    {
        s_protoAlloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_CDECL, "malloc", PIN_PARG(size_t), PIN_PARG_END());
        s_init       = true;
    }

    RTN stdAllocRtn = RTN_FindByName(img, "malloc");
    if (RTN_Valid(stdAllocRtn) && RTN_IsSafeForProbedReplacement(stdAllocRtn))
    {
        RTN_ReplaceSignatureProbed(stdAllocRtn, AFUNPTR(MallocProbe), IARG_PROTOTYPE, s_protoAlloc, IARG_ORIG_FUNCPTR,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_CONTEXT, IARG_RETURN_IP, IARG_END);
    }
    else if (RTN_Valid(stdAllocRtn))
    { // This is workaround for mantis 4588. When mantis is handled this code need to be addressed.
        fprintf(fp, "Replacement not safe\n");
        fflush(fp);
    }
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        exit(0);
    }

    fp = fopen("probecdecl.outfile", "w");

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

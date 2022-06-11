/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

/*
This tool, in conjunction with the passthru_context_xmm_regs_app, verifies that a CONTEXT * passed to analysis
routine and used from that analysis routine, without change, in the PIN_CallApplication API, has the correct
values in the xmm registers, when the app xmm registers are in the spill area
*/

extern "C" unsigned int xmmInitVals[];
unsigned int xmmInitVals[64];
extern "C" int SetXmmScratchesFun();

#ifdef TARGET_IA32E
#define NUM_XMM_REGS 16
#else
#define NUM_XMM_REGS 8
#endif

KNOB< BOOL > KnobUseIargConstContext(KNOB_MODE_WRITEONCE, "pintool", "const_context", "0", "use IARG_CONST_CONTEXT");

CHAR fpContextSpaceForFpConextFromPin[FPSTATE_SIZE + FPSTATE_ALIGNMENT];

BOOL replacementRoutineCalled          = FALSE;
BOOL instrumentedBeforeReplacedXmmRegs = FALSE;

VOID REPLACE_ReplacedXmmRegs(CONTEXT* context, THREADID tid, AFUNPTR originalFunction)
{
    replacementRoutineCalled = TRUE;
    printf("TOOL in REPLACE_ReplacedXmmRegs\n");
    printf("TOOL Calling original application function\n");
    fflush(stdout);
    PIN_CallApplicationFunction(context, tid, CALLINGSTD_DEFAULT, (AFUNPTR)originalFunction, NULL, PIN_PARG_END());
    // no return from the application function
    ASSERTX(0);
}

VOID Image(IMG img, void* v)
{
    RTN rtn = RTN_FindByName(img, "ReplacedXmmRegs");
    if (RTN_Valid(rtn))
    {
        PROTO proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "ReplacedXmmRegs", PIN_PARG_END());
        RTN_ReplaceSignature(rtn, AFUNPTR(REPLACE_ReplacedXmmRegs), IARG_PROTOTYPE, proto,
                             (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_ORIG_FUNCPTR,
                             IARG_END);
        PROTO_Free(proto);
        printf("TOOL found and replaced ReplacedXmmRegs\n");
        fflush(stdout);
    }
    rtn = RTN_FindByName(img, "BeforeReplacedXmmRegs");
    if (RTN_Valid(rtn))
    {   // insert an analysis call that sets the xmm scratch registers just before the call to
        // the replaced function
        instrumentedBeforeReplacedXmmRegs = TRUE;
        printf("TOOL found BeforeReplacedXmmRegs\n");
        fflush(stdout);
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)SetXmmScratchesFun, IARG_END);
        RTN_Close(rtn);
    }
}

static void OnExit(INT32, VOID*)
{
    if (!replacementRoutineCalled)
    {
        printf("***TOOL Error !replacementRoutineCalled\n");
        fflush(stdout);
        PIN_ExitProcess(1);
    }
    if (!instrumentedBeforeReplacedXmmRegs)
    {
        printf("***TOOL Error !instrumentedBeforeReplacedXmmRegs\n");
        fflush(stdout);
        PIN_ExitProcess(1);
    }
}

int main(int argc, char** argv)
{
    // initialize memory area used to set values in ymm regs
    for (int i = 0; i < 64; i++)
    {
        xmmInitVals[i] = 0xdeadbeef;
    }
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    IMG_AddInstrumentFunction(Image, 0);

    PIN_AddFiniFunction(OnExit, 0);

    PIN_StartProgram();
    return 0;
}

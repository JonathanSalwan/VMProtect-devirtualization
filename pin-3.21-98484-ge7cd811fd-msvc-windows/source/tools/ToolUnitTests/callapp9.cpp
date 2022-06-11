/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// The tool callapp9.cpp and application inner.c showed a bug with the way REG_INST_G0
// is handled.  That register should have the value "1" when executing code
// from PIN_CallApplicationFunction() and the value "0" when executing other
// code. However, after execution returns from the replaced function, REG_INST_G0
// continued to have the value "1". The correct behavior is is for REG_INST_G0
// to have the value "0" when it returns from the replacement function.
//
// The correct output is:
// At Inner G0=0
// Calling replaced Replaced()
// REPLACE_Replaced: REG_INST_G0=1
// At Inner G0=1
// REPLACE_Replaced: REG_INST_G0=1
// Returning from replaced Replaced()
// i=2
// At Inner G0=0

#include <stdio.h>
#include "pin.H"

KNOB< BOOL > KnobUseIargConstContext(KNOB_MODE_WRITEONCE, "pintool", "const_context", "0", "use IARG_CONST_CONTEXT");

static REG scratchReg;

int REPLACE_Replaced(CONTEXT* context, THREADID tid, AFUNPTR func)
{
    int ret;

    fprintf(stderr, "Calling replaced Replaced()\n");
    fflush(stderr);

    CONTEXT writableContext, *ctxt;
    if (KnobUseIargConstContext)
    { // need to copy the ctxt into a writable context
        PIN_SaveContext(context, &writableContext);
        ctxt = &writableContext;
    }
    else
    {
        ctxt = context;
    }

    PIN_SetContextReg(ctxt, scratchReg, 1);
    fprintf(stderr, "REPLACE_Replaced: REG_INST_G0=0x%lx\n", (unsigned long)PIN_GetContextReg(ctxt, scratchReg));
    fflush(stderr);

    PIN_CallApplicationFunction(ctxt, tid, CALLINGSTD_DEFAULT, func, NULL, PIN_PARG(int), &ret, PIN_PARG_END());

    fprintf(stderr, "REPLACE_Replaced: REG_INST_G0=0x%lx\n", (unsigned long)PIN_GetContextReg(ctxt, scratchReg));
    fprintf(stderr, "Returning from replaced Replaced()\n");
    fflush(stderr);

    return ret;
}

void AtInner(ADDRINT g0)
{
    fprintf(stderr, "At Inner G0=%d\n", (int)g0);
    fflush(stderr);
}

VOID Image(IMG img, void* v)
{
    RTN rtn = RTN_FindByName(img, "Replaced");
    if (RTN_Valid(rtn))
    {
        PROTO proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "Replaced", PIN_PARG_END());
        RTN_ReplaceSignature(rtn, AFUNPTR(REPLACE_Replaced), IARG_PROTOTYPE, proto,
                             (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_ORIG_FUNCPTR,
                             IARG_END);
        PROTO_Free(proto);
    }

    rtn = RTN_FindByName(img, "Inner");
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(AtInner), IARG_REG_VALUE, scratchReg, IARG_END);
        RTN_Close(rtn);
    }
}

VOID OnThread(THREADID threadIndex, CONTEXT* ctxt, INT32 flags, VOID* v) { PIN_SetContextReg(ctxt, scratchReg, 0); }

int main(int argc, char** argv)
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    scratchReg = PIN_ClaimToolRegister();
    if (!REG_valid(scratchReg))
    {
        fprintf(stderr, "Cannot allocate a scratch register.\n");
        return 1;
    }

    PIN_AddThreadStartFunction(OnThread, 0);
    IMG_AddInstrumentFunction(Image, 0);

    PIN_StartProgram();
    return 0;
}

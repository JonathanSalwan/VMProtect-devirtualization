/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Test that we can call PIN_CallApplicationFunction() when there is a pending signal.
 */

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "pin.H"

static VOID OnImage(IMG, VOID*);
static VOID OnFoo(CONTEXT*, THREADID);

BOOL FoundBar = FALSE;
ADDRINT BarAddr;

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    IMG_AddInstrumentFunction(OnImage, 0);

    PIN_StartProgram();
    return 0;
}

static VOID OnImage(IMG img, VOID*)
{
#ifdef TARGET_MAC
    RTN rtn = RTN_FindByName(img, "_PIN_TEST_FOO");
#else
    RTN rtn = RTN_FindByName(img, "PIN_TEST_FOO");
#endif
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(OnFoo), IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        RTN_Close(rtn);
    }

#ifdef TARGET_MAC
    rtn = RTN_FindByName(img, "_PIN_TEST_BAR");
#else
    rtn = RTN_FindByName(img, "PIN_TEST_BAR");
#endif
    if (RTN_Valid(rtn))
    {
        BarAddr  = RTN_Address(rtn);
        FoundBar = TRUE;
    }
}

static VOID OnFoo(CONTEXT* ctxt, THREADID tid)
{
    // Sending a signal to ourself here will cause the signal to remain pending in Pin until this analysis
    // routine returns.
    //
    kill(getpid(), SIGUSR1);

    if (!FoundBar)
    {
        fprintf(stderr, "Unable to find PIN_TEST_BAR()\n");
        PIN_ExitProcess(1);
    }

    PIN_CallApplicationFunction(ctxt, tid, CALLINGSTD_DEFAULT, AFUNPTR(BarAddr), NULL, PIN_PARG_END());
    PIN_CallApplicationFunction(ctxt, tid, CALLINGSTD_DEFAULT, AFUNPTR(BarAddr), NULL, PIN_PARG_END());
}

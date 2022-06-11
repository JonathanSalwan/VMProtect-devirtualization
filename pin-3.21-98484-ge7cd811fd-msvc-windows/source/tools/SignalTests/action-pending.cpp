/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that PIN_IsActionPending() returns TRUE when there
 * is a pending signal.
 */

#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "pin.H"

static VOID OnImage(IMG, VOID*);
static VOID Check(THREADID);
static void OnExit(INT32, VOID*);

static BOOL DidCheck = FALSE;

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    IMG_AddInstrumentFunction(OnImage, 0);
    PIN_AddFiniFunction(OnExit, 0);

    PIN_StartProgram();
    return 0;
}

static VOID OnImage(IMG img, VOID*)
{
#if defined(TARGET_MAC)
    RTN rtn = RTN_FindByName(img, "_HandlerIsEstablished");
#else
    RTN rtn = RTN_FindByName(img, "HandlerIsEstablished");
#endif
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(Check), IARG_THREAD_ID, IARG_END);
        RTN_Close(rtn);
    }
}

static VOID Check(THREADID tid)
{
    DidCheck = TRUE;
    kill(getpid(), SIGUSR1);
    if (!PIN_IsActionPending(tid))
    {
        std::cerr << "There should be a signal pending\n";
        std::exit(1);
    }
}

static VOID OnExit(INT32, VOID*)
{
    if (!DidCheck)
    {
        std::cerr << "Test error: didn't do check\n";
        std::exit(1);
    }
}

/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test only runs with the application "action-pending-app.cpp".  It helps
 * test the PIN_IsActionPending() API.
 */

#include <iostream>
#include <cstdlib>
#include <sched.h>
#include "pin.H"

static void InstrumentRtn(RTN, VOID*);
static void OnExit(INT32, VOID*);
static void DoWait(CONTEXT*, THREADID);
static void DoSignal();

static volatile int Signal  = 0;
static BOOL FoundToolWait   = FALSE;
static BOOL FoundToolSignal = FALSE;

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    RTN_AddInstrumentFunction(InstrumentRtn, 0);
    PIN_AddFiniFunction(OnExit, 0);
    PIN_StartProgram();
    return 0;
}

static void InstrumentRtn(RTN rtn, VOID*)
{
    if (RTN_Name(rtn) == "ToolWait")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoWait), IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        FoundToolWait = TRUE;
        RTN_Close(rtn);
    }
    if (RTN_Name(rtn) == "ToolSignal")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoSignal), IARG_END);
        FoundToolSignal = TRUE;
        RTN_Close(rtn);
    }
}

static void OnExit(INT32, VOID*)
{
    if (!FoundToolWait || !FoundToolSignal)
    {
        std::cout << "Couldn't add instrumentation routines" << std::endl;
        exit(1);
    }
}

static void DoWait(CONTEXT* ctxt, THREADID tid)
{
    std::cout << "Tool is waiting" << std::endl;
    while (Signal == 0)
    {
        if (PIN_IsActionPending(tid)) PIN_ExecuteAt(ctxt);
        sched_yield();
    }
    std::cout << "Tool done waiting" << std::endl;
}

static void DoSignal()
{
    std::cout << "Tool about to signal" << std::endl;
    Signal = 1;
}

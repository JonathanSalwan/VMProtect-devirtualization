/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool is meant to run on the application "pc-change-async.cpp".
 */

#include <iostream>
#include <cstdlib>
#include "pin.H"

BOOL FoundBreakpointFunction = FALSE;
BOOL FoundBreakpointLocation = FALSE;
BOOL FoundOneFunction        = FALSE;
BOOL FoundTwoFunction        = FALSE;
BOOL FoundOneRtn             = FALSE;
BOOL AllowBreakpoint         = FALSE;

ADDRINT BreakpointFunction;
ADDRINT BreakpointLocation;
ADDRINT OneFunction;
ADDRINT TwoFunction;

PIN_LOCK Lock;

static VOID OnImage(IMG, VOID*);
static VOID InstrumentRtn(RTN rtn, VOID*);
static VOID AtOne();
static BOOL Intercept(THREADID, DEBUGGING_EVENT, CONTEXT*, VOID*);
static VOID OnExit(INT32, VOID*);

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();
    PIN_InitLock(&Lock);

    IMG_AddInstrumentFunction(OnImage, 0);
    RTN_AddInstrumentFunction(InstrumentRtn, 0);
    PIN_InterceptDebuggingEvent(DEBUGGING_EVENT_BREAKPOINT, Intercept, 0);
    PIN_InterceptDebuggingEvent(DEBUGGING_EVENT_ASYNC_BREAK, Intercept, 0);
    PIN_AddFiniFunction(OnExit, 0);
    PIN_StartProgram();
    return 0;
}

static VOID OnImage(IMG img, VOID*)
{
    for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym))
    {
        if (SYM_Name(sym) == "Breakpoint")
        {
            FoundBreakpointFunction = TRUE;
            BreakpointFunction      = SYM_Address(sym);
        }
        if (SYM_Name(sym) == "BreakpointLocation")
        {
            FoundBreakpointLocation = TRUE;
            BreakpointLocation      = SYM_Address(sym);
        }
        if (SYM_Name(sym) == "One")
        {
            FoundOneFunction = TRUE;
            OneFunction      = SYM_Address(sym);
        }
        if (SYM_Name(sym) == "Two")
        {
            FoundTwoFunction = TRUE;
            TwoFunction      = SYM_Address(sym);
        }
    }
}

static VOID InstrumentRtn(RTN rtn, VOID*)
{
    if (RTN_Name(rtn) == "One")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(AtOne), IARG_END);
        FoundOneRtn = TRUE;
        RTN_Close(rtn);
    }
}

static VOID AtOne()
{
    if (!AllowBreakpoint)
    {
        PIN_GetLock(&Lock, 1);
        std::cout << "Breakpoint is allowed" << std::endl;
        PIN_ReleaseLock(&Lock);
    }

    // When the main thread reaches the One() function, allow the child thread to trigger
    // the breakpoint.  The main thread will continue executing this instruction in a loop
    // until we intercept the DEBUGGING_EVENT_ASYNC_BREAK event below.
    //
    AllowBreakpoint = TRUE;
}

static BOOL Intercept(THREADID tid, DEBUGGING_EVENT eventType, CONTEXT* ctxt, VOID*)
{
    if (eventType == DEBUGGING_EVENT_BREAKPOINT)
    {
        // When the child thread reaches the breakpoint in Breakpoint(), wait for the main
        // thread to reach the One() function.  If the main thread is not there yet, squash the
        // breakpoint and move the PC back to the start of the Breakpoint() function.  This will
        // delay a while and then re-trigger the breakpoint.
        //
        ADDRINT pc = PIN_GetContextReg(ctxt, REG_INST_PTR);
        if (pc == BreakpointLocation && !AllowBreakpoint)
        {
            PIN_SetContextReg(ctxt, REG_INST_PTR, BreakpointFunction);
            PIN_GetLock(&Lock, 1);
            std::cout << "Squashing breakpoint at 0x" << std::hex << pc << " on thread " << std::dec << tid << std::endl;
            PIN_ReleaseLock(&Lock);
            return FALSE;
        }

        PIN_GetLock(&Lock, 1);
        std::cout << "Stopping at breakpoint at 0x" << std::hex << pc << " on thread " << std::dec << tid << std::endl;
        PIN_ReleaseLock(&Lock);
        return TRUE;
    }

    if (eventType == DEBUGGING_EVENT_ASYNC_BREAK)
    {
        // When the child thread triggers the breakpoint, we should be at the One() function.
        // Change the PC to the Two() function, which is the point of this test.  We want to
        // make sure Pin properly handles the change of PC in this case.
        //
        ADDRINT pc = PIN_GetContextReg(ctxt, REG_INST_PTR);
        if (pc == OneFunction)
        {
            PIN_SetContextReg(ctxt, REG_INST_PTR, TwoFunction);
            PIN_GetLock(&Lock, 1);
            std::cout << "Changing ASYNC BREAK PC to Two() on thread " << std::dec << tid << std::endl;
            PIN_ReleaseLock(&Lock);
            return TRUE;
        }

        // If the PC is not at the One() function, the child thread has probably hit some breakpoint
        // other than the one at Breakpoint().  (E.g. an internal breakpoint set by GDB.)  Don't
        // change the PC in such a case.
        //
        PIN_GetLock(&Lock, 1);
        std::cout << "ASYNC_BREAK at 0x" << std::hex << pc << " on thread " << std::dec << tid << std::endl;
        PIN_ReleaseLock(&Lock);
        return TRUE;
    }

    PIN_GetLock(&Lock, 1);
    std::cout << "FAILURE: Unexpected debugging event type" << std::endl;
    PIN_ReleaseLock(&Lock);
    std::exit(1);
}

static VOID OnExit(INT32, VOID*)
{
    if (!FoundBreakpointFunction)
    {
        std::cout << "FAILURE: Did not find Breakpoint() SYM" << std::endl;
        std::exit(1);
    }
    if (!FoundBreakpointLocation)
    {
        std::cout << "FAILURE: Did not find BreakpointLocation SYM" << std::endl;
        std::exit(1);
    }
    if (!FoundOneFunction)
    {
        std::cout << "FAILURE: Did not find One() SYM" << std::endl;
        std::exit(1);
    }
    if (!FoundTwoFunction)
    {
        std::cout << "FAILURE: Did not find Two() SYM" << std::endl;
        std::exit(1);
    }
    if (!FoundOneRtn)
    {
        std::cout << "FAILURE: Did not find One() RTN" << std::endl;
        std::exit(1);
    }
}

/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This test tool verifies that Pin on Windows correctly handles thread suspension
 * and termination system calls.  The tool must be run with the "suspend_app_win"
 * application.
 */

#include <set>
#include "pin.H"

using std::set;

enum DOFLUSH_STATE
{
    DOFLUSH_STATE_NONE,
    DOFLUSH_STATE_INSTRUMENTATION,
    DOFLUSH_STATE_ANALYSIS,
    DOFLUSH_STATE_FLUSH_CALLED
};

static volatile DOFLUSH_STATE flushState = DOFLUSH_STATE_NONE;
static ADDRINT doFlushAddress            = 0;
static volatile bool flushHappened       = false;

/*!
 * RTN analysis routines.
 */
static VOID OnIsToolPresent(ADDRINT addrIsTool)
{
    volatile int* pIsTool = reinterpret_cast< volatile int* >(addrIsTool);
    // Set the <isTool> flag in the application
    *pIsTool = 1;
}

static VOID OnSleepInTool(ADDRINT addrInTool)
{
    volatile int* pInTool = reinterpret_cast< volatile int* >(addrInTool);
    // Set the <inTool> flag in the application while sleeping (1 second).
    *pInTool = 1;
    OS_Sleep(1000);
    *pInTool = 0;
}

static VOID OnDoFlush()
{
    static bool flushVerified = false;
    switch (flushState)
    {
        case DOFLUSH_STATE_NONE:
            ASSERTX(0);
            break;
        case DOFLUSH_STATE_INSTRUMENTATION:
            flushState = DOFLUSH_STATE_ANALYSIS;
            break;
        case DOFLUSH_STATE_ANALYSIS:
            flushState    = DOFLUSH_STATE_FLUSH_CALLED;
            flushVerified = false;
            PIN_RemoveInstrumentation();
            break;
        case DOFLUSH_STATE_FLUSH_CALLED:
            ASSERTX(!flushVerified);
            ASSERTX(flushHappened);
            flushVerified = true;
            break;
    }
}

static VOID OnCheckFlush(ADDRINT addrCodeCacheFlushed)
{
    const unsigned int sleepLimit         = 5 * 60 * 1000; // 5 minutes in milliseconds
    const unsigned int microSleepDuration = 10;
    const unsigned int numMicroSleeps     = sleepLimit / microSleepDuration;
    for (unsigned int i = 0; i < numMicroSleeps; ++i)
    {
        if (flushHappened)
        {
            *reinterpret_cast< volatile int* >(addrCodeCacheFlushed) = 1;
            break;
        }
        OS_Sleep(microSleepDuration);
    }
}

/*!
 * RTN instrumentation routine.
 */
static VOID InstrumentRoutine(RTN rtn, VOID*)
{
    if (RTN_Name(rtn) == "IsToolPresent")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(OnIsToolPresent), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        RTN_Close(rtn);
    }
    else if (RTN_Name(rtn) == "SleepInTool")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(OnSleepInTool), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        RTN_Close(rtn);
    }
    else if (RTN_Name(rtn) == "DoFlush")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(OnDoFlush), IARG_END);
        RTN_Close(rtn);
        doFlushAddress = RTN_Address(rtn);
    }
    else if (RTN_Name(rtn) == "CheckFlush")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(OnCheckFlush), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        RTN_Close(rtn);
    }
}

static VOID Trace(TRACE trace, VOID* v)
{
    if (TRACE_Address(trace) != doFlushAddress) return;
    switch (flushState)
    {
        case DOFLUSH_STATE_NONE:
            flushState = DOFLUSH_STATE_INSTRUMENTATION;
            break;
        case DOFLUSH_STATE_INSTRUMENTATION:
            ASSERTX(0);
            break;
        case DOFLUSH_STATE_ANALYSIS:
            flushState = DOFLUSH_STATE_INSTRUMENTATION;
            break;
        case DOFLUSH_STATE_FLUSH_CALLED:
            flushHappened = true;
            break;
    }
}

/*!
 * The main procedure of the tool.
 */
int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    RTN_AddInstrumentFunction(InstrumentRoutine, 0);
    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_StartProgram();
    return 0;
}

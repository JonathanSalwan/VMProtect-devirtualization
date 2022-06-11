/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/**
 * This tool tests the breakpoint handling APIs.
 *
 * The test takes control over a breakpoint at the ToolControlled function.
 *
 * At first the simulates the breakpoint using PIN_ApplicationBreakpoint, 
 * then receives the breakpoint delete notifaction (which requires the tool
 * to stop causing breakpoints), then the breakpoint is re-inserted. After
 * that, it returns control back to PinADX.
 */
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "pin.H"

KNOB< BOOL > KnobWaitForDebugger(KNOB_MODE_WRITEONCE, "pintool", "wait_for_debugger", "0", "Wait for debugger to connect");

ADDRINT _toolControlledFuncAddr = 0;
BOOL _isJobDone                 = FALSE;
BOOL _toolShouldStop            = FALSE;

static BOOL BreakpointHandler(ADDRINT addr, UINT size, BOOL added, VOID* data);
static VOID InstrumentImg(IMG img, VOID* data);
static ADDRINT IsAtBreakpoint(ADDRINT);
static VOID ControlledBreakpoint(CONTEXT*, THREADID);
static VOID ReActivateControlledBreakpoint();

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    PIN_AddBreakpointHandler(BreakpointHandler, 0);
    IMG_AddInstrumentFunction(InstrumentImg, 0);

    PIN_StartProgram();
    return 0;
}

static BOOL BreakpointHandler(ADDRINT addr, UINT size, BOOL insert, VOID* data)
{
    if (_isJobDone) return FALSE;

    // Only interested in my controlled function
    //
    if (RTN_FindNameByAddress(addr) != "ToolControlled") return FALSE;

    _toolControlledFuncAddr = addr;

    // If it is a new breakpoint, we should stop.
    //
    _toolShouldStop = insert;

    return TRUE;
}

static VOID InstrumentImg(IMG img, VOID* data)
{
    if (!IMG_IsMainExecutable(img)) return;

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            if (RTN_Name(rtn) == "ToolControlled")
            {
                _toolControlledFuncAddr = RTN_Address(rtn);
                RTN_Open(rtn);
                for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
                {
                    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IsAtBreakpoint, IARG_INST_PTR, IARG_END);
                    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)ControlledBreakpoint, IARG_CONST_CONTEXT, IARG_THREAD_ID,
                                       IARG_END);
                }
                RTN_Close(rtn);
            }
            if (RTN_Name(rtn) == "ReActivateToolControlled")
            {
                RTN_Open(rtn);
                RTN_InsertCall(rtn, IPOINT_AFTER, AFUNPTR(ReActivateControlledBreakpoint), IARG_END);
                RTN_Close(rtn);
            }
        }
    }
}

static ADDRINT IsAtBreakpoint(ADDRINT pc) { return (pc == _toolControlledFuncAddr); }

static VOID ControlledBreakpoint(CONTEXT* ctxt, THREADID tid)
{
    if (_toolShouldStop)
    {
        _toolShouldStop = FALSE;
        PIN_ApplicationBreakpoint(ctxt, tid, KnobWaitForDebugger.Value(), "Stopped by the tool.");
    }
}

static VOID ReActivateControlledBreakpoint()
{
    _isJobDone      = TRUE;
    _toolShouldStop = FALSE;
    PIN_ResetBreakpointAt(_toolControlledFuncAddr);
}

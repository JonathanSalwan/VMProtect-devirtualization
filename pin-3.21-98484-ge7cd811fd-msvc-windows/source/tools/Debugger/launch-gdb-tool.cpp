/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test triggers an application breakpoint from an analysis
 * routine.  Before triggering the breakpoint, the analysis
 * routine asks Pin if a debugger is already attached.  If not,
 * it launches GDB, and GDB attaches to Pin.
 */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include "pin.H"

KNOB< std::string > KnobOut(KNOB_MODE_WRITEONCE, "pintool", "o", "",
                            "Output file where debugger connection information is written");
KNOB< UINT32 > KnobTimeout(KNOB_MODE_WRITEONCE, "pintool", "timeout", "", "Timeout period to wait for GDB to connect (seconds)");
KNOB< BOOL > KnobUseIargConstContext(KNOB_MODE_WRITEONCE, "pintool", "const_context", "0", "use IARG_CONST_CONTEXT");

static void OnRtn(RTN, VOID*);
static void DoBreakpoint(CONTEXT*, THREADID);
static bool LaunchGdb();

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    RTN_AddInstrumentFunction(OnRtn, 0);

    PIN_StartProgram();
    return 0;
}

static void OnRtn(RTN rtn, VOID*)
{
    // We want to stop at a debugger breakpoint at the "Inner" function.
    //
    if (RTN_Name(rtn) == "Inner")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoBreakpoint), (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT,
                       // IARG_CONST_CONTEXT has much lower overhead
                       // than IARG_CONTEX for passing the CONTEXT*
                       // to the analysis routine. Note that IARG_CONST_CONTEXT
                       // passes a read-only CONTEXT* to the analysis routine
                       IARG_THREAD_ID, IARG_END);
        RTN_Close(rtn);
    }
}

static void DoBreakpoint(CONTEXT* ctxt, THREADID tid)
{
    static bool IsReplayedInstruction = false;
    static bool IsDebuggerLaunched    = false;

    // When resuming from a breakpoint, we re-execute the instruction at
    // the breakpoint.  Skip over the breakpoint in this case.
    //
    if (IsReplayedInstruction)
    {
        IsReplayedInstruction = false;
        return;
    }

    // If the debugger isn't launched yet, launch it now.
    //
    if (PIN_GetDebugStatus() == DEBUG_STATUS_UNCONNECTED)
    {
        // This is a sanity check to make sure PIN_GetDebugStatus() doesn't
        // have a bug.  Once the debugger is launched, the status should
        // no longer be DEBUG_STATUS_UNCONNECTED.
        //
        if (IsDebuggerLaunched)
        {
            std::cerr << "Got wrong Debug Status from Pin" << std::endl;
            exit(1);
        }

        if (!LaunchGdb())
        {
            std::cerr << "GDB did not connect within the timeout period." << std::endl;
            exit(1);
        }
        IsDebuggerLaunched = true;
    }

    IsReplayedInstruction = true;

    // Stop at a debugger breakpoint.
    //
    PIN_ApplicationBreakpoint(ctxt, tid, TRUE, "Tool stopped at breakpoint");
    /* does not return */
}

static bool LaunchGdb()
{
    // Ask Pin for the TCP port that it is listening on.  GDB needs to
    // connect to this port via the "target remote" command.
    //
    DEBUG_CONNECTION_INFO info;
    PIN_GetDebugConnectionInfo(&info);
    std::ofstream outf(KnobOut.Value().c_str());
    outf << "set remotetimeout " << KnobTimeout.Value() << "\n";
    outf << "target remote :" << info._tcpServer._tcpPort << "\n";
    outf.close();

    // The "makefile" should launch GDB when we write the output file above.
    // Wait for GDB to launch and connect.
    //
    return PIN_WaitForDebuggerToConnect(1000 * KnobTimeout.Value());
}

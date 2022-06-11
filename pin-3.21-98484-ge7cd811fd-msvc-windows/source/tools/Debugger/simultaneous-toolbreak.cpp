/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool is intended to run with the test applications "thread-unix.cpp"
 * or "thread-windows.cpp".  It raises a PinADX tool breakpoint whenever a
 * thread calls the GlobalFunction() function.  There is also a knob that
 * will ensure that all threads report their breakpoint at the same time.
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include "pin.H"
#include "atomic.hpp"
#include "tool_macros.h"

KNOB< unsigned > KnobSyncForThreads(KNOB_MODE_WRITEONCE, "pintool", "sync_for_threads", "0",
                                    "Add synchronization to ensure that all tool breakpoints happen at the same time.  "
                                    "The argument to this knob tells the number of threads in the application "
                                    "that will call the GlobalFunction() function.");
KNOB< BOOL > KnobWaitForDebugger(KNOB_MODE_WRITEONCE, "pintool", "wait_for_debugger", "0",
                                 "If no debugger is connected, wait for one to connect when the breakpoints trigger.");
KNOB< std::string > KnobPort(KNOB_MODE_WRITEONCE, "pintool", "port", "", "Output file where TCP information is written");
KNOB< BOOL > KnobUseIargConstContext(KNOB_MODE_WRITEONCE, "pintool", "const_context", "0", "use IARG_CONST_CONTEXT");

static bool OnCommand(THREADID, CONTEXT*, const std::string&, std::string*, VOID*);
static BOOL ParseThreadId(const std::string&, THREADID*);
static void InstrumentRtn(RTN, VOID*);
static void DoBreakpoint(THREADID, CONTEXT*);
static ADDRINT ClearSkipPc();
static void OnExit(INT32, VOID*);

static REG RegSkipOne;
static BOOL FoundWorker                  = FALSE;
static volatile unsigned BreakpointCount = 0;
static PIN_SEMAPHORE ReadyToDoBreakpoint;

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();
    PIN_SemaphoreInit(&ReadyToDoBreakpoint);
    RegSkipOne = PIN_ClaimToolRegister();
    if (!REG_valid(RegSkipOne))
    {
        std::cerr << "Unable to allocate tool register" << std::endl;
        return 1;
    }

    if (!KnobPort.Value().empty())
    {
        DEBUG_CONNECTION_INFO info;
        if (!PIN_GetDebugConnectionInfo(&info))
        {
            std::cerr << "Error from PIN_GetDebugConnectionInfo()" << std::endl;
            return 1;
        }
        if (info._type != DEBUG_CONNECTION_TYPE_TCP_SERVER)
        {
            std::cerr << "Unexpected connection type from PIN_GetDebugConnectionInfo()" << std::endl;
            return 1;
        }

        std::ofstream out(KnobPort.Value().c_str());
        out << std::dec << info._tcpServer._tcpPort;
    }

    PIN_AddDebugInterpreter(OnCommand, 0);
    RTN_AddInstrumentFunction(InstrumentRtn, 0);
    PIN_AddFiniFunction(OnExit, 0);
    PIN_StartProgram();
    return 0;
}

static bool OnCommand(THREADID, CONTEXT*, const std::string& cmd, std::string* reply, VOID*)
{
    if (cmd == "list-pending")
    {
        unsigned nPendingToolbreaks = 0;
        UINT32 nThreads             = PIN_GetStoppedThreadCount();
        for (UINT32 i = 0; i < nThreads; i++)
        {
            THREADID tid = PIN_GetStoppedThreadId(i);
            if (PIN_GetStoppedThreadPendingToolBreakpoint(tid, 0)) nPendingToolbreaks++;
        }

        std::ostringstream os;
        os << "There are " << std::dec << nPendingToolbreaks << " pending breakpoints:\n";
        for (UINT32 i = 0; i < nThreads; i++)
        {
            std::string msg;
            THREADID tid = PIN_GetStoppedThreadId(i);
            if (PIN_GetStoppedThreadPendingToolBreakpoint(tid, &msg)) os << "  " << std::dec << tid << ": " << msg;
        }
        *reply = os.str();
        return true;
    }

    if (cmd.find("squash-pending ") == 0)
    {
        std::string what   = cmd.substr(sizeof("squash-pending ") - 1);
        unsigned nSquashed = 0;

        THREADID tid = 0;
        if (what == "all")
        {
            UINT32 nThreads = PIN_GetStoppedThreadCount();
            for (UINT32 i = 0; i < nThreads; i++)
            {
                tid = PIN_GetStoppedThreadId(i);
                if (PIN_ChangePendingToolBreakpointOnStoppedThread(tid, TRUE, "")) nSquashed++;
            }
        }
        else if (ParseThreadId(what, &tid))
        {
            if (PIN_ChangePendingToolBreakpointOnStoppedThread(tid, TRUE, "")) nSquashed++;
        }
        else
        {
            *reply = "Invalid parameter '" + what + "'\n";
            return true;
        }

        std::ostringstream os;
        os << "Squashed " << std::dec << nSquashed << " breakpoints\n";
        *reply = os.str();
        return true;
    }

    if (cmd.find("append-pending ") == 0)
    {
        std::string rest   = cmd.substr(sizeof("append-pending ") - 1);
        size_t pos         = rest.find(' ');
        std::string what   = (pos == std::string::npos) ? "" : rest.substr(0, pos);
        std::string append = (pos == std::string::npos) ? "" : rest.substr(pos + 1);
        if (!append.empty() && append[append.size() - 1] == '\n') append.erase(append.size() - 1, 1);

        unsigned nChanged = 0;
        THREADID tid      = 0;
        if (what == "all")
        {
            UINT32 nThreads = PIN_GetStoppedThreadCount();
            for (UINT32 i = 0; i < nThreads; i++)
            {
                tid = PIN_GetStoppedThreadId(i);
                std::string msg;
                if (PIN_GetStoppedThreadPendingToolBreakpoint(tid, &msg))
                {
                    size_t pos = msg.size();
                    if (!msg.empty() && msg[pos - 1] == '\n') pos--;
                    msg.insert(pos, " " + append);
                    if (PIN_ChangePendingToolBreakpointOnStoppedThread(tid, FALSE, msg)) nChanged++;
                }
            }
        }
        else if (ParseThreadId(what, &tid))
        {
            std::string msg;
            if (PIN_GetStoppedThreadPendingToolBreakpoint(tid, &msg))
            {
                size_t pos = msg.size();
                if (!msg.empty() && msg[pos - 1] == '\n') pos--;
                msg.insert(pos, " " + append);
                if (PIN_ChangePendingToolBreakpointOnStoppedThread(tid, FALSE, msg)) nChanged++;
            }
        }
        else
        {
            *reply = "Invalid parameter '" + what + "'\n";
            return true;
        }

        std::ostringstream os;
        os << "Changed " << std::dec << nChanged << " breakpoints\n";
        *reply = os.str();
        return true;
    }

    return false;
}

static BOOL ParseThreadId(const std::string& str, THREADID* val)
{
    std::istringstream is(str);

    THREADID id;
    is >> std::dec >> id;
    if (is.fail() || !is.eof()) return FALSE;
    *val = id;
    return TRUE;
}

static void InstrumentRtn(RTN rtn, VOID*)
{
    if (RTN_Name(rtn) == C_MANGLE("GlobalFunction"))
    {
        RTN_Open(rtn);
        INS ins = RTN_InsHeadOnly(rtn);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoBreakpoint), IARG_THREAD_ID,
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT,
                       // IARG_CONST_CONTEXT has much lower overhead
                       // than IARG_CONTEX for passing the CONTEXT*
                       // to the analysis routine. Note that IARG_CONST_CONTEXT
                       // passes a read-only CONTEXT* to the analysis routine
                       IARG_END);
        if (INS_IsValidForIpointAfter(ins))
            INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(ClearSkipPc), IARG_RETURN_REGS, RegSkipOne, IARG_END);
        if (INS_IsValidForIpointTakenBranch(ins))
            INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(ClearSkipPc), IARG_RETURN_REGS, RegSkipOne, IARG_END);
        FoundWorker = TRUE;
        RTN_Close(rtn);
    }
}

static void DoBreakpoint(THREADID tid, CONTEXT* context)
{
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

    ADDRINT skipPc = PIN_GetContextReg(ctxt, RegSkipOne);
    ADDRINT pc     = PIN_GetContextReg(ctxt, REG_INST_PTR);
    if (skipPc == pc) return;
    PIN_SetContextReg(ctxt, RegSkipOne, pc);

    // If this knob is specified, add synchronization that ensures that each thread
    // reports the tool breakpoint at the same time.  Synchonization like this is
    // not generally safe, but this tool is only run on a simple test program, and
    // the synchronization is safe in that case.
    //
    if (KnobSyncForThreads.Value())
    {
        if (ATOMIC::OPS::Increment< unsigned >(&BreakpointCount, 1) + 1 >= KnobSyncForThreads.Value())
        {
            PIN_SemaphoreSet(&ReadyToDoBreakpoint);
            if (KnobWaitForDebugger.Value()) std::cout << "Waiting for debugger to attach" << std::endl;
        }
        PIN_SemaphoreWait(&ReadyToDoBreakpoint);
    }

    // When the process stops in the debugger, all threads will be simultaneously
    // stopped here.
    //
    std::ostringstream os;
    os << "Stopping in Worker Thread " << std::dec << tid << "\n";
    PIN_ApplicationBreakpoint(ctxt, tid, KnobWaitForDebugger.Value(), os.str());
}

static ADDRINT ClearSkipPc() { return 0; }

static void OnExit(INT32, VOID*)
{
    if (!FoundWorker)
    {
        std::cout << "FAILURE: Couldn't add instrumentation routines" << std::endl;
        std::exit(1);
    }
}

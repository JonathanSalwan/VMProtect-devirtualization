/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*
 * Execute up to the first system call, and then replay it without re-executing it
 * to demonstrate that PIN_ReplaySyscall does what it says on the can.
 */

/* ===================================================================== */
/*! @file
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
using std::cerr;
using std::endl;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "replay_syscall.out", "specify file name");

ofstream TraceFile;
/* ===================================================================== */

static INT32 Usage()
{
    cerr << "This pin tool checks the operation of the PIN_ReplaySyscall function\n"
            "By executing a test code until it has executed a system call, then\n"
            "replaying the system call, and checking that the SYSCALL_ENTRY and SYSCALL_EXIT\n"
            " callbacks are called."
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

static int syscallCount  = 0;
static int callbackCount = 0;

static CONTEXT savedContext;
SYSCALL_STANDARD savedStd;

// Function called before the system call. We call this twice to make sure that
// we see the callbacks in the correct order in all cases.
static VOID syscallBefore(THREADID tid, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    UINT32 callNumber = UINT32(ADDRINT(v) & 0xffffffff);

    TraceFile << callNumber << " syscallBefore (" << PIN_GetSyscallNumber(ctxt, std) << ")" << endl;

    if (callNumber == 1 && syscallCount == 0)
    {
        PIN_SaveContext(ctxt, &savedContext);
        savedStd = std;
    }

    callbackCount++;
}

static VOID syscallAfter(THREADID tid, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    UINT32 callNumber = UINT32(ADDRINT(v) & 0xffffffff);

    // Note that the result will be wrong teh second time around, since we don't bother to
    // save and restore the context after the system call. That's OK, this test isn't about
    // that, which is up to a replayig tool anyway, but about whether the callbacks get made...
    TraceFile << callNumber << " syscallAfter: result (" << PIN_GetSyscallReturn(ctxt, std) << ")" << endl;
    if (callNumber == 2) syscallCount++;

    callbackCount++;
}

static VOID syscallInstructionInstrumentation(THREADID tid, CONTEXT* ctxt)
{
    if (syscallCount == 1)
    {
        TraceFile << "Replaying system call; actual call is " << PIN_GetSyscallNumber(ctxt, savedStd) << "\n";
        PIN_ReplaySyscallEntry(tid, &savedContext, savedStd);
        PIN_ReplaySyscallExit(tid, &savedContext, savedStd);
        if (callbackCount == 8)
        {
            TraceFile.close();
            exit(0);
        }
        else
        {
            TraceFile << "***Bad callback count " << callbackCount << " should be 8\n";
            TraceFile.close();
            exit(1);
        }
    }
}

static void instrumentInstruction(INS ins, VOID* v)
{
    if (!INS_IsSyscall(ins)) return;

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)syscallInstructionInstrumentation, IARG_THREAD_ID, IARG_CONTEXT, IARG_END);
}

int main(int argc, CHAR* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    TraceFile.open(KnobOutputFile.Value().c_str());

    PIN_AddSyscallEntryFunction(syscallBefore, (VOID*)1);
    PIN_AddSyscallEntryFunction(syscallBefore, (VOID*)2);
    PIN_AddSyscallExitFunction(syscallAfter, (VOID*)1);
    PIN_AddSyscallExitFunction(syscallAfter, (VOID*)2);

    INS_AddInstrumentFunction(instrumentInstruction, 0);

    PIN_StartProgram();
}

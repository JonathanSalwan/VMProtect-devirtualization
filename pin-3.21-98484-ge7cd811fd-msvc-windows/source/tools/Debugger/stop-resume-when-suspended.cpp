/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* When the first instructions of the application is executed, the application is counted as
 * Stopped in the debugger.
 * In this test we try to stop and resume the debugged application during that time to check
 * that everything's working as expected.
 */
#include <fstream>
#include "pin.H"

KNOB< std::string > KnobOut(KNOB_MODE_WRITEONCE, "pintool", "o", "stop-resume-when-suspended.out", "Output file");

static std::ofstream Out;
static PIN_LOCK pinLock;
static volatile THREADID mainThread = INVALID_THREADID;

// Check that stopping and resuming the program from the same thread works
static VOID SuspendResume(THREADID tid)
{
    // Main thread must be set by now
    ASSERTX(mainThread != INVALID_THREADID);

    // On Windows we might be called from APC created threads that we
    // don't expect. Be sure to filter them
    if (tid != mainThread) return;

    if (PIN_StopApplicationThreads(tid))
    {
        // Resume threads only if stop succeeded
        PIN_ResumeApplicationThreads(tid);
    }
}

static VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    PIN_GetLock(&pinLock, threadid);

    // Record the first thread that started and call it the main thread
    if (INVALID_THREADID == mainThread) mainThread = threadid;
    PIN_ReleaseLock(&pinLock);
}

static void OnExit(INT32, VOID*)
{
    Out << "OnExit" << std::endl;
    Out.close();
}

static VOID Image(IMG img, VOID* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            RTN_Open(rtn);
            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SuspendResume), IARG_THREAD_ID, IARG_END);
            }
            RTN_Close(rtn);
        }
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    Out.open(KnobOut.Value().c_str());

    // Initialize the lock
    PIN_InitLock(&pinLock);

    // Register ThreadStart to be called when a thread starts.
    PIN_AddThreadStartFunction(ThreadStart, NULL);

    // Register Instruction to be called to instrument instructions.
    IMG_AddInstrumentFunction(Image, NULL);

    // Register Fini to be called when the application exits.
    PIN_AddFiniFunction(OnExit, 0);

    // Start the program, never returns
    PIN_StartProgram();
    return 0;
}

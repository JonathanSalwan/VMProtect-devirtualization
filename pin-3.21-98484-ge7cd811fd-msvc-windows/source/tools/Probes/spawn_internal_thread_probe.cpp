/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * A tool that creates internal Pin thread
 * and verifies that the thread is finished gracefully.
 * Probe mode.
 */

#include <cstdlib>
#include <string>
#include <iostream>
#include "tool_macros.h"
#include "pin.H"
using std::string;

using std::cerr;
using std::endl;

// UID of the internal thread. It is created in the application thread by the
// main() tool's procedure.
PIN_THREAD_UID intThreadUid;

/*!
 * Print out the error message and exit the process.
 */
static void AbortProcess(const string& msg, unsigned long code)
{
    cerr << "Test aborted: " << msg << " with code " << code << endl;
    PIN_WriteErrorMessage(msg.c_str(), 1002, PIN_ERR_FATAL, 0);
    PIN_ExitProcess(1);
}

BOOL isInternalThreadCreated = FALSE;

BOOL MyIsInternalThreadCreated() { return isInternalThreadCreated; }

BOOL isAppTerminated = FALSE;

/*!
 * Internal tool's thread. It is created in the application thread by the
 * main() tool's procedure.
 */
VOID IntThread(VOID* arg)
{
    isInternalThreadCreated = TRUE;

    while (!isAppTerminated)
    {
        PIN_Sleep(10);
    }

    cerr << "in IntThread going to finish thread gracefully." << endl;

    PIN_ExitThread(0);
}

/*!
 * Process exit callback (unlocked).
 */
VOID PrepareForFini()
{
    BOOL waitStatus;
    INT32 threadExitCode;

    // Notify internal thread to finish.
    isAppTerminated = TRUE;

    // First, wait for termination of the main internal thread. When this thread exits,
    // all secondary internal threads are already created and, so <uidSet> can be safely
    // accessed without lock.
    waitStatus = PIN_WaitForThreadTermination(intThreadUid, 1000, &threadExitCode);
    if (!waitStatus)
    {
        AbortProcess("PIN_WaitForThreadTermination(RootThread) failed", 0);
    }
    if (0 != threadExitCode)
    {
        AbortProcess("Tool's thread exited abnormally", 0);
    }

    cerr << "Tool's thread finished successfully." << endl;
}

typedef void (*FUNCPTR)(int status);
static void (*pf_exit)(int status);

VOID MyExit(int arg)
{
    PrepareForFini();

    (pf_exit)(arg);
}

VOID ImageLoad(IMG img, VOID* v)
{
    RTN rtn;
#if defined(TARGET_WINDOWS)
    rtn = RTN_FindByName(img, "exit");
#else
    rtn = RTN_FindByName(img, "_exit");
#endif

    if (RTN_Valid(rtn))
    {
        if (RTN_IsSafeForProbedReplacementEx(rtn, PROBE_MODE_ALLOW_RELOCATION))
        {
            pf_exit = (FUNCPTR)RTN_ReplaceProbedEx(rtn, PROBE_MODE_ALLOW_RELOCATION, AFUNPTR(MyExit));
        }
        else
        {
            AbortProcess("ImageLoad: Pin can't replace _exit()", 0);
        }
    }

    if (IMG_IsMainExecutable(img))
    {
        rtn = RTN_FindByName(img, C_MANGLE("IsInternalThreadCreated"));
        if (RTN_Valid(rtn))
        {
            if (RTN_IsSafeForProbedReplacementEx(rtn, PROBE_MODE_ALLOW_RELOCATION))
            {
                RTN_ReplaceProbedEx(rtn, PROBE_MODE_ALLOW_RELOCATION, AFUNPTR(MyIsInternalThreadCreated));
            }
            else
            {
                AbortProcess("ImageLoad: Pin can't replace IsInternalThreadCreated()", 0);
            }
        }
        else
        {
            AbortProcess("ImageLoad: Pin can't find IsInternalThreadCreated()", 0);
        }
    }
}

/*!
 * The main procedure of the tool.
 */
int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    IMG_AddInstrumentFunction(ImageLoad, NULL);

    // Spawn the main internal thread. When this thread starts it spawns all other internal threads.
    THREADID intThreadId = PIN_SpawnInternalThread(IntThread, NULL, 0, &intThreadUid);
    if (INVALID_THREADID == intThreadId)
    {
        AbortProcess("PIN_SpawnInternalThread(intThread) failed", 0);
    }

    // Never returns
    PIN_StartProgramProbed();
    return 1;
}

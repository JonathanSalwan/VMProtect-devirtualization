/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * A tool that creates internal Pin thread
 * and verifies that the thread is finished gracefully.
 */

#include "pin.H"
#include <string>
#include <iostream>

namespace WIND
{
#include <windows.h>
}

using std::cerr;
using std::endl;
using std::string;

/*!
 * Global variables.
 */

// UID of the internal thread. It is created in the application thread by the
// main() tool's procedure.
PIN_THREAD_UID intThreadUid;
// Pointer to TID of internal thread. Imported from application.
unsigned int* pTid;

//==========================================================================
// Utilities
//==========================================================================

/*!
 * Print out the error message and exit the process.
 */
static void AbortProcess(const string& msg, unsigned long code)
{
    cerr << "Test aborted: " << msg << " with code " << code << endl;
    PIN_WriteErrorMessage(msg.c_str(), 1002, PIN_ERR_FATAL, 0);
}

/*!
 * Internal tool's thread. It is created in the application thread by the
 * main() tool's procedure.
 */
static VOID IntThread(VOID* arg)
{
    WIND::HMODULE exeHandle = WIND::GetModuleHandle(NULL);
    pTid                    = (unsigned int*)WIND::GetProcAddress(exeHandle, "tid");

    // Sets current TID in imported variable, makes it available to application.
    *pTid = WIND::GetCurrentThreadId();

    // tid value is reset in Fini.
    while (*pTid != 0)
    {
        PIN_Sleep(10);
    }

    // Finishes gracefully if application doesn't harm the thread.
    PIN_ExitThread(0);
}

//==========================================================================
// Instrumentation callbacks
//==========================================================================
/*!
 * Process exit callback (unlocked).
 */
static VOID PrepareForFini(VOID* v)
{
    BOOL waitStatus;
    INT32 threadExitCode;

    // Notify internal thread to finish.
    *pTid = 0;

    // First, wait for termination of the main internal thread. When this thread exits,
    // all secondary internal threads are already created and, so <uidSet> can be safely
    // accessed without lock.
    waitStatus = PIN_WaitForThreadTermination(intThreadUid, 1000, &threadExitCode);
    if (!waitStatus)
    {
        AbortProcess("PIN_WaitForThreadTermination(RootThread) failed", 0);
    }
    if (threadExitCode != 0)
    {
        AbortProcess("Tool's thread exited abnormally", threadExitCode);
    }

    cerr << "Tool's thread finished successfully." << endl;
}

/*!
 * Process exit callback.
 */
static VOID Fini(INT32 code, VOID* v)
{
    if (code != 0)
    {
        AbortProcess("Application exited abnormally", code);
    }
}

/*!
 * The main procedure of the tool.
 */
int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    PIN_AddPrepareForFiniFunction(PrepareForFini, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Spawn the main internal thread. When this thread starts it spawns all other internal threads.
    THREADID intThreadId = PIN_SpawnInternalThread(IntThread, NULL, 0, &intThreadUid);
    if (intThreadId == INVALID_THREADID)
    {
        AbortProcess("PIN_SpawnInternalThread(intThread) failed", 0);
    }

    // Never returns
    PIN_StartProgram();
    return 0;
}

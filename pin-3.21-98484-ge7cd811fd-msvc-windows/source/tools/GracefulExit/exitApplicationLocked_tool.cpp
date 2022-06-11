/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool tests for graceful exit when a tool calls PIN_ExitApplication while holding the client lock. PIN_ExitApplication is
 * called from the thread-start callback of the secondary thread, thus terminating the application.
 *
 */

#include <cstdio>
#include <cstring>
#include <cassert>
#include <fstream>
#include "pin.H"
using std::endl;
using std::string;

using std::ofstream;

/**************************************************
 * Global variables                               *
 **************************************************/
// TLS key for saving each thread's OS thread ID.
// In the ThreadStart callback, each thread stores its tid in the TLS and prints it to the startsOut file.
// In the threadFini callback, each thread prints the tid as it appears in the TLS to the finisOut file.
// We expect this value to correspond to the exiting thread (for which the callback is given) and not the
// OS thread ID of the actual thread executing the callback.
static TLS_KEY tidKey;

// Knobs for defining the output filenames. We need one for the thread start callbacks
// and one for the thread fini callbacks.
KNOB< string > KnobThreadsStartsFile(KNOB_MODE_WRITEONCE, "pintool", "startsfile", "threadStarts.out",
                                     "specify file name for thread start callbacks output");
KNOB< string > KnobThreadsFinisFile(KNOB_MODE_WRITEONCE, "pintool", "finisfile", "threadFinis.out",
                                    "specify file name for thread fini callbacks output");

// Output file streams
ofstream startsOut;
ofstream finisOut;

// Counter for verifying that the application's fini function is called after all the thread finis.
// This is done in the makefile:
// 1. Verify that all started threads (threads that executed the ThreadStart callback) have terminated
//    gracefully by comparing the startsOut file and the finisOut file.
// 2. Check that numOfActiveThreads is 0. If it is positive, it means that the Fini callback was called
//    before one of the ThreadFini callbacks. It should not be negative.
volatile int numOfActiveThreads = 0;

// Counter for verifying that all expected threads were created. Its value is printed in the Fini callback
// and checked in the makefile.
volatile int totalNumOfThreads = 0;

/**************************************************
 * Utility functions                              *
 **************************************************/
// Retrieve a tid stored in the TLS.
static OS_THREAD_ID* GetTLSData(THREADID threadIndex)
{
    return static_cast< OS_THREAD_ID* >(PIN_GetThreadData(tidKey, threadIndex));
}

/**************************************************
 * Analysis routines                              *
 **************************************************/
static VOID ThreadStart(THREADID threadIndex, CONTEXT* c, INT32 flags, VOID* v)
{
    ++numOfActiveThreads;
    ++totalNumOfThreads;
    OS_THREAD_ID* tidData = new OS_THREAD_ID(PIN_GetTid());
    PIN_SetThreadData(tidKey, tidData, threadIndex);
    startsOut << *tidData << endl;
    fprintf(stderr, "TOOL: <%d> thread start, active: %d\n", *tidData, numOfActiveThreads);
    fflush(stderr);
    if (threadIndex != 0)
    {
        PIN_LockClient(); // take the client lock recursively
        fprintf(stderr, "TOOL: <%d> is now calling PIN_ExitApplication with the client lock held.\n", *tidData);
        fflush(stderr);
        PIN_ExitApplication(0); // never returns
    }
}

static VOID ThreadFini(THREADID threadIndex, CONTEXT const* c, INT32 code, VOID* v)
{
    --numOfActiveThreads;
    OS_THREAD_ID* tidData = GetTLSData(threadIndex);
    finisOut << *tidData << endl;
    fprintf(stderr, "TOOL: <%d> thread fini, fini: %d\n", *tidData, numOfActiveThreads);
    fflush(stderr);
}

static VOID Fini(INT32 code, VOID* v)
{
    OS_THREAD_ID tid = PIN_GetTid();
    fprintf(stderr, "TOOL: <%d> fini function %d %d\n", tid, numOfActiveThreads, totalNumOfThreads);
    fflush(stderr);
}

/**************************************************
 * Main function                                  *
 **************************************************/
int main(INT32 argc, CHAR** argv)
{
    // Initialize Pin and TLS
    PIN_InitSymbols();
    PIN_Init(argc, argv);
    tidKey = PIN_CreateThreadDataKey(0);

    // Set up output files
    startsOut.open(KnobThreadsStartsFile.Value().c_str());
    finisOut.open(KnobThreadsFinisFile.Value().c_str());

    // Register callbacks
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Start running the application
    PIN_StartProgram(); // Never returns

    return 0;
}

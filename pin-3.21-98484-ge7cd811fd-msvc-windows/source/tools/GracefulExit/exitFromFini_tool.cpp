/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool tests for correctness when a tool calls PIN_ExitApplication/PIN_ExitProcess from within a thread-fini
 * or application-fini callback. See exitFromFini_app.cpp for complete details.
 */

#include <cstdio>
#include <cstring>
#include <cassert>
#include <fstream>
#include <set>
#include "pin.H"
using std::endl;
using std::string;

using std::ofstream;
using std::set;

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
// When the test is done, we sort and compare them and expect them to be identical. This verifies access
// to the correct TLS during the thread fini callbacks.
KNOB< string > KnobThreadsStartsFile(KNOB_MODE_WRITEONCE, "pintool", "startsfile", "threadStarts.out",
                                     "specify file name for thread start callbacks output");
KNOB< string > KnobThreadsFinisFile(KNOB_MODE_WRITEONCE, "pintool", "finisfile", "threadFinis.out",
                                    "specify file name for thread fini callbacks output");

// Knob for specifying the scenario to be tested.
KNOB< int > KnobScenario(KNOB_MODE_WRITEONCE, "pintool", "scenario", "0", "specify the current scenario to be checked (1-6)");

// Output file streams
ofstream startsOut;
ofstream finisOut;

THREADID myThread = INVALID_THREADID;
set< THREADID > appThreads;

// Scenario
int scenario = 0;

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
    if (myThread == INVALID_THREADID)
    {
        myThread = threadIndex;
    }
    ++numOfActiveThreads;
    ++totalNumOfThreads;
    OS_THREAD_ID* tidData = new OS_THREAD_ID(PIN_GetTid());
    PIN_SetThreadData(tidKey, tidData, threadIndex);
    if (myThread == threadIndex)
    {
        startsOut << *tidData << endl;
    }
    fprintf(stderr, "TOOL: <%d> thread start, active: %d\n", *tidData, numOfActiveThreads);
    fflush(stderr);
}

static VOID ThreadFini(THREADID threadIndex, CONTEXT const* c, INT32 code, VOID* v)
{
    --numOfActiveThreads;
    OS_THREAD_ID* tidData = GetTLSData(threadIndex);
    if (myThread == threadIndex || appThreads.find(threadIndex) != appThreads.end())
    {
        finisOut << *tidData << endl;
    }
    fprintf(stderr, "TOOL: <%d> thread fini, fini: %d\n", *tidData, numOfActiveThreads);
    fflush(stderr);
    switch (scenario)
    {
        case 2:
            if (threadIndex != 0) break; // in scenario 2, call PIN_ExitApplication for the main thread only.
        case 1:
            fprintf(stderr, "TOOL: <%d> calling PIN_ExitApplication from thread fini for thread %d\n", *tidData, threadIndex);
            fflush(stderr);
            PIN_ExitApplication(0);
        case 5:
            if (threadIndex != 0) break; // in scenario 5, call PIN_ExitProcess for the main thread only.
        case 4:
            fprintf(stderr, "TOOL: <%d> calling PIN_ExitProcess from thread fini for thread %d\n", *tidData, threadIndex);
            fflush(stderr);
            PIN_ExitProcess(0);
        default:
            break;
    }
}

static VOID Fini(INT32 code, VOID* v)
{
    OS_THREAD_ID tid = PIN_GetTid();
    fprintf(stderr, "TOOL: <%d> fini function %d %d\n", tid, numOfActiveThreads, totalNumOfThreads);
    fflush(stderr);
    switch (scenario)
    {
        case 3:
            fprintf(stderr, "TOOL: <%d> calling PIN_ExitApplication from application fini\n", tid);
            fflush(stderr);
            PIN_ExitApplication(0);
        case 6:
            fprintf(stderr, "TOOL: <%d> calling PIN_ExitProcess from application fini\n", tid);
            fflush(stderr);
            PIN_ExitProcess(0);
        default:
            break;
    }
}

static VOID AppThreadStart(THREADID threadIndex)
{
    OS_THREAD_ID* tidData = GetTLSData(threadIndex);
    startsOut << *tidData << endl;
    appThreads.insert(threadIndex);
}

static VOID InstrumentRtn(RTN rtn, VOID*)
{
    if (PIN_UndecorateSymbolName(RTN_Name(rtn), UNDECORATION_NAME_ONLY) == "DoNewThread")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(AppThreadStart), IARG_THREAD_ID, IARG_END);
        RTN_Close(rtn);
    }
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

    // Verify scenario
    if (KnobScenario.Value() < 1 || KnobScenario > 6)
    {
        fprintf(stderr, "TOOL ERROR: Invalid scenario specified\n%s\n", KnobScenario.StringKnobSummary().c_str());
        return 1;
    }

    scenario = KnobScenario.Value();

    // Register callbacks
    RTN_AddInstrumentFunction(InstrumentRtn, NULL);
    PIN_AddThreadStartFunction(ThreadStart, NULL);
    PIN_AddThreadFiniFunction(ThreadFini, NULL);
    PIN_AddFiniFunction(Fini, NULL);

    // Start running the application
    PIN_StartProgram(); // Never returns

    return 0;
}

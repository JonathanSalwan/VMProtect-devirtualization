/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool checks that all thread-fini and application fini callbacks are delivered upon program
 * termination in the correct order.
 * Various scenarios are checked depending on the application being run with this tool.
 */

#include <cstdio>
#include <cstring>
#include <fstream>
#include <ctime>
#include <set>
#include "pin.H"
using std::endl;
using std::set;
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
// When the test is done, we sort and compare them and expect them to be identical. This verifies access
// to the correct TLS during the thread fini callbacks.
KNOB< string > KnobThreadsStartsFile(KNOB_MODE_WRITEONCE, "pintool", "startsfile", "threadStarts.out",
                                     "specify file name for thread start callbacks output");
KNOB< string > KnobThreadsFinisFile(KNOB_MODE_WRITEONCE, "pintool", "finisfile", "threadFinis.out",
                                    "specify file name for thread fini callbacks output");
KNOB< string > KnobOutFile(KNOB_MODE_WRITEONCE, "pintool", "o", "tool.out", "specify file name for general tool output");

// Output file streams
ofstream startsOut;
ofstream finisOut;
FILE* outfile;

// Flag for signaling the tool's internal exit thread that it can call PIN_ExitApplication.
volatile bool releaseExitThread = false;

// Counter for verifying that the application's fini function is called after all the thread finis.
// This is done in the makefile:
// 1. Verify that all started threads (threads that executed the ThreadStart callback) have terminated
//    gracefully by comparing the startsOut file and the finisOut file.
// 2. Check that numOfActiveThreads is 0. If it is positive, it means that the Fini callback was called
//    before one of the ThreadFini callbacks. It should not be negative.
volatile int numOfActiveThreads = 0;

volatile int numOfFinishedThreads = 0;

// Counter for verifying that all expected threads were created. Its value is printed in the Fini callback
// and checked in the makefile.
volatile int totalNumOfThreads = 0;

THREADID myThread = INVALID_THREADID;
set< THREADID > appThreads;
PIN_LOCK pinLock;

static struct _ATEXIT
{
    ~_ATEXIT()
    {
        // We assume that stdout is not yet closed by application.
        fprintf(stdout, "Tool unloaded.\n");
        fflush(stdout);
    }
} s1;

/**************************************************
 * Function declarations                          *
 **************************************************/
static void InternalThreadMain(void* v);

/**************************************************
 * Utility functions                              *
 **************************************************/
// Retrieve a tid sotred in the TLS.
static OS_THREAD_ID* GetTLSData(THREADID threadIndex)
{
    return static_cast< OS_THREAD_ID* >(PIN_GetThreadData(tidKey, threadIndex));
}

// Creates an internal thread for the tool.
static void CreateToolThread()
{
    if (PIN_SpawnInternalThread(InternalThreadMain, NULL, 0, NULL) == INVALID_THREADID)
    {
        fprintf(outfile, "TOOL: <%d> Unable to spawn internal thread. Killing the test!\n", PIN_GetTid());
        fflush(outfile);
        PIN_ExitProcess(101);
    }
}

// Calls exit application either directly (by the application thread) or indirectly (by releasing the internal thread).
static void callExitApplication(bool appThread)
{
    if (appThread)
    {
        PIN_GetLock(&pinLock, PIN_GetTid());
        fprintf(outfile, "TOOL: <%d> Application thread calling PIN_ExitApplication()\n", PIN_GetTid());
        fflush(outfile);
        PIN_ReleaseLock(&pinLock);
        PIN_ExitApplication(0);
    }
    else
    {
        PIN_GetLock(&pinLock, PIN_GetTid());
        fprintf(outfile, "TOOL: <%d> Releasing the internal exit thread\n", PIN_GetTid());
        fflush(outfile);
        PIN_ReleaseLock(&pinLock);
        releaseExitThread = true;
    }
}

static VOID AppThreadStart(THREADID threadIndex)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    ++numOfActiveThreads;
    ++totalNumOfThreads;
    OS_THREAD_ID* tidData = new OS_THREAD_ID(PIN_GetTid());
    PIN_SetThreadData(tidKey, tidData, threadIndex);
    startsOut << *tidData << endl;
    fprintf(outfile, "TOOL: <%d> thread start, active: %d\n", *tidData, numOfActiveThreads);
    fflush(outfile);
    appThreads.insert(threadIndex);
    PIN_ReleaseLock(&pinLock);
}

/**************************************************
 * Analysis routines                              *
 **************************************************/

static VOID ThreadStart(THREADID threadIndex, CONTEXT* c, INT32 flags, VOID* v)
{
    if (myThread == INVALID_THREADID)
    {
        // Assume the first ThreadStart event is for main thread.
        if (threadIndex != 0)
        {
            PIN_ExitProcess(103);
        }
        myThread = threadIndex;
        AppThreadStart(threadIndex);
    }
}

static VOID ThreadFini(THREADID threadIndex, CONTEXT const* c, INT32 code, VOID* v)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    if (appThreads.find(threadIndex) != appThreads.end())
    {
        appThreads.erase(appThreads.find(threadIndex));
        --numOfActiveThreads;
        ++numOfFinishedThreads;
        OS_THREAD_ID* tidData = GetTLSData(threadIndex);
        finisOut << *tidData << endl;
        fprintf(outfile, "TOOL: <%d> thread fini, fini: %d\n", *tidData, numOfActiveThreads);
        fflush(outfile);
    }
    PIN_ReleaseLock(&pinLock);
}

static VOID Fini(INT32 code, VOID* v)
{
    fprintf(outfile, "TOOL: <%d> fini function %d %d\n", PIN_GetTid(), numOfActiveThreads, totalNumOfThreads);
    fprintf(outfile, "TOOL: <%d> fini function %d %d\n", PIN_GetTid(), numOfFinishedThreads, totalNumOfThreads);
    fflush(outfile);
    fclose(outfile);
    startsOut.close();
    finisOut.close();
}

/**************************************************
 * Instrumentation routines                       *
 **************************************************/
static VOID OnImage(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_FindByName(img, "doExit");
        if (RTN_Valid(rtn))
        {
            PIN_GetLock(&pinLock, PIN_GetTid());
            fprintf(outfile, "TOOL: <%d> Found doExit routine\n", PIN_GetTid());
            fflush(outfile);
            PIN_ReleaseLock(&pinLock);
            RTN_Open(rtn);
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)callExitApplication, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            RTN_Close(rtn);
        }
        else
        {
            PIN_GetLock(&pinLock, PIN_GetTid());
            fprintf(outfile, "TOOL: <%d> Unable to find the doExit routine. Killing the test!\n", PIN_GetTid());
            fflush(outfile);
            PIN_ReleaseLock(&pinLock);
            PIN_ExitProcess(102);
        }
        rtn = RTN_FindByName(img, "DoNewThread");
        if (RTN_Valid(rtn))
        {
            RTN_Open(rtn);
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)AppThreadStart, IARG_THREAD_ID, IARG_END);
            RTN_Close(rtn);
        }
    }
}

/**************************************************
 * Main functions                                 *
 **************************************************/
// Internal thread's main function
void InternalThreadMain(void* v)
{
    time_t internal_th_start = time(NULL);
    PIN_GetLock(&pinLock, PIN_GetTid());
    fprintf(outfile, "TOOL: <%d> Internal thread was created succesfully.\n", PIN_GetTid());
    fflush(outfile);
    PIN_ReleaseLock(&pinLock);
    int timeout = 600;
    while (!releaseExitThread && --timeout > 0)
    { // wait here until the application requests to exit
        PIN_Yield();
        PIN_Sleep(1000); // 1 sec
    }

    PIN_GetLock(&pinLock, PIN_GetTid());
    fprintf(outfile, "TOOL: timeout = %d, seconds since starting internal thread=%.f\n", timeout,
            difftime(time(NULL), internal_th_start));
    if (timeout <= 0)
    {
        fprintf(outfile, "TOOL: <%d> Internal thread got time out after 10 minutes - exiting with an error.\n", PIN_GetTid());
        fflush(outfile);
        PIN_ReleaseLock(&pinLock);
        PIN_ExitProcess(102);
    }
    else
    {
        fprintf(outfile, "TOOL: <%d> Internal thread calling PIN_ExitApplication().\n", PIN_GetTid());
        fflush(outfile);
        PIN_ReleaseLock(&pinLock);
        PIN_ExitApplication(0);
    }
}

// Tool's main function.
// The last parameter is expected to be a number which signifies which test is being run.
int main(INT32 argc, CHAR** argv)
{
    PIN_InitLock(&pinLock);
    // Initialize Pin and TLS
    PIN_InitSymbols();
    PIN_Init(argc, argv);
    tidKey = PIN_CreateThreadDataKey(NULL);

    // Set up output files
    startsOut.open(KnobThreadsStartsFile.Value().c_str());
    finisOut.open(KnobThreadsFinisFile.Value().c_str());
    outfile = fopen(KnobOutFile.Value().c_str(), "w");

    // Register callbacks
    PIN_AddThreadStartFunction(ThreadStart, NULL);
    IMG_AddInstrumentFunction(OnImage, NULL);
    PIN_AddThreadFiniFunction(ThreadFini, NULL);
    PIN_AddFiniFunction(Fini, NULL);

    // test "1" calls PIN_ExitApplication by a tool internal thread. We need to create that thread only in this case.
    if (strcmp(argv[argc - 1], "1") == 0)
    {
        CreateToolThread();
    }

    // Start running the application
    PIN_StartProgram(); // Never returns

    return 0;
}

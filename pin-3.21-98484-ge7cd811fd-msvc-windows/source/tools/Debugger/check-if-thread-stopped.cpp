/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool should instrument the thread-{unix,windows} application, and should test
 * the PIN_IsThreadStoppedInDebugger() functionality using these steps:
 * 1. All started threads call GlobalFunction() which is instrumented by this tool.
 * 2. We order all threads according to an arbitrary order.
 * 3. We active each thread one by one according to the order from step 2 and in each thread we do:
 *   a. Check that all the threads that were activated before are stopped in the debugger
 *      (using PIN_IsThreadStoppedInDebugger()).
 *   b. Check that all the threads that weren't activated yet aren't stopped in the debugger
 *      (using PIN_IsThreadStoppedInDebugger()).
 *   c. Stop the activated thread in the debugger (by PIN_ApplicationBreakpoint()) and wake
 *      up next thread in the order
  */
#include <pin.H>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <time.h>

#ifdef TARGET_WINDOWS
namespace WIND
{
#include <Windows.h>
}
#define YIELD WIND::SwitchToThread
#else
#include <sched.h>
#define YIELD sched_yield
#endif

KNOB< std::string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "check-if-thread-stopped.out", "Output file");
KNOB< unsigned > KnobThreads(KNOB_MODE_WRITEONCE, "pintool", "threads", "4", "Number of threads");

// This semaphore is set after all the application threads reached
// the same (predefined) execution point
PIN_SEMAPHORE SemAllThreadStarted;

// The thread IDs which were created and instrumented in the application
// The order of the threads in this vector is the order in which the
// threads will operate
std::vector< THREADID > VecThreadIds;

// This mutex protects the insertion to VecThreadIds from multiple threads
PIN_MUTEX MtxVecThreadIds;

// The index (into VecThreadIds) of the where the thread ID of the current
// active thread
volatile unsigned ActiveThreadIndex = 0;

// This lock acquired by the active thread and released when the active
// thread is finished
PIN_MUTEX MtxActiveThread;

// Output file string for this tool's log file
std::ofstream Out;

// This variable assumes the value 'true' when all threads were started
volatile bool AllThreadsWereStarted = false;

/* =====================================================================
 * Print the content of VecThreadIds with the chosen order
 * ===================================================================== */
void PrintAllThreadIDs()
{
    Out << "All started threads IDs [" << std::dec;
    for (size_t i = 0; i < VecThreadIds.size(); i++)
    {
        if (i > 0) Out << ", ";

        Out << VecThreadIds[i];
    }
    Out << "]" << std::endl;
}

/* =====================================================================
 * Called on DoBreakpoint() for each thread.
 * Waits for all the threads to reach this point, filling the vector of
 * the created thread IDs, and then release all the threads.
 * ===================================================================== */
static void WaitForAllThreadsToStart(THREADID tid)
{
    PIN_MutexLock(&MtxVecThreadIds);
    VecThreadIds.push_back(tid);
    if (VecThreadIds.size() >= KnobThreads.Value())
    {
        PrintAllThreadIDs();
        AllThreadsWereStarted = true;
        PIN_SemaphoreSet(&SemAllThreadStarted);
    }
    PIN_MutexUnlock(&MtxVecThreadIds);
    PIN_SemaphoreWait(&SemAllThreadStarted);
}

/* =====================================================================
 * Wait for the turn of the thread denoted by 'tid' to do its checking
 * ===================================================================== */
void WaitForThisThreadToBeActive(THREADID tid)
{
    // Waits for the turn of this thread
    while (VecThreadIds[ActiveThreadIndex] != tid)
    {
        PIN_MutexUnlock(&MtxActiveThread);
        YIELD();
        PIN_MutexLock(&MtxActiveThread);
    }

    // At this point it's the turn of the current thread to do the checking
    // the previous thread release the mutex MtxActiveThread.
    // We still need to wait (for at most 10 seconds) for the previous thread
    // to stop at the debugger
    time_t startWaitingTime = time(NULL);
    if (ActiveThreadIndex > 0)
    {
        THREADID prevTid = VecThreadIds[ActiveThreadIndex - 1];
        while (!PIN_IsThreadStoppedInDebugger(prevTid))
        {
            ASSERT((time(NULL) - startWaitingTime) < 10,
                   "Timeout waiting for thread " + hexstr(prevTid) + " to stop in debugger");
            YIELD();
        }
    }
}

/* =====================================================================
 * Called on each thread when it calls GlobalFunction()
 * ===================================================================== */
static void DoBreakpoint(THREADID tid, CONTEXT* context)
{
    // If all thread were started then this is the second time we're entering
    // this function for the thread 'tid'.
    // We enter this function twice for the same thread because we hit the same
    // instrumentation we the debugger was returned from the break-point that
    // we mock to the same address. In this case we need to return and don't
    // repeat the test (unless we'll end up with an infinite test).
    if (AllThreadsWereStarted) return;

    WaitForAllThreadsToStart(tid);
    // All threads are synchronized to this point

    PIN_MutexLock(&MtxActiveThread);

    WaitForThisThreadToBeActive(tid);

    Out << "Performing check on thread ID " << tid << std::endl;

    for (unsigned i = 0; i < KnobThreads.Value(); i++)
    {
        THREADID otherTid = VecThreadIds[i];
        if (i < ActiveThreadIndex)
        {
            // All threads that were already active should be stopped in the debugger
            ASSERT(PIN_IsThreadStoppedInDebugger(otherTid), "Thread " + decstr(otherTid) + " should be stopped");
        }
        else
        {
            // All threads that weren't already active should be running
            ASSERT(!PIN_IsThreadStoppedInDebugger(otherTid), "Thread " + decstr(otherTid) + " shouldn't be stopped");
        }
    }

    ActiveThreadIndex++;
    PIN_MutexUnlock(&MtxActiveThread);

    // Stop this thread in the debugger
    // Note that because we add the instrumentation at IPOINT_AFTER, the program counter
    // at 'context' will not lead us to run this instrumentation function again
    PIN_ApplicationBreakpoint(context, tid, FALSE, "Stopping in Worker Thread " + decstr(tid) + "\n");
}

/* =====================================================================
 * Called upon image load to instrument the function GlobalFunction
 * ===================================================================== */
static void Image(IMG img, VOID*)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_FindByName(img, "GlobalFunction");
        ASSERT(RTN_Valid(rtn), "Failed to find GlobalFunction() in main application image");

        RTN_Open(rtn);
        INS ins = RTN_InsHeadOnly(rtn);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoBreakpoint), IARG_THREAD_ID, IARG_CONTEXT, IARG_END);
        RTN_Close(rtn);
    }
}

/* =====================================================================
 * Called upon program finish
 * ===================================================================== */
static void OnExit(INT32, VOID*)
{
    PIN_SemaphoreFini(&SemAllThreadStarted);
    PIN_MutexFini(&MtxVecThreadIds);
    PIN_MutexFini(&MtxActiveThread);

    ASSERT(!PIN_IsThreadStoppedInDebugger(VecThreadIds[0]),
           "Terminated thread with TID " + decstr(VecThreadIds[0]) + " shouldn't be appeared as stopped");

    Out << "Finished" << std::endl;
    Out.close();
}

/* =====================================================================
 * Called upon bad command line argument
 * ===================================================================== */
INT32 Usage()
{
    std::cerr << "This pin tool tests the functionality of PIN_IsThreadStoppedInDebugger()" << std::endl;

    std::cerr << KNOB_BASE::StringKnobSummary();

    std::cerr << std::endl;

    return -1;
}

/* =====================================================================
 * Entry point for the tool
 * ===================================================================== */
int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    PIN_InitSymbols();

    PIN_SemaphoreInit(&SemAllThreadStarted);
    PIN_MutexInit(&MtxVecThreadIds);
    PIN_MutexInit(&MtxActiveThread);

    Out.open(KnobOutputFile.Value().c_str());
    ASSERT(Out, "Failed to open file " + KnobOutputFile.Value());

    IMG_AddInstrumentFunction(Image, 0);

    PIN_AddFiniFunction(OnExit, 0);

    // Never returns
    PIN_StartProgram();
    return 0;
}
/* ===================================================================== */
/* eof */
/* ===================================================================== */

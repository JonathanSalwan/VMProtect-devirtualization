/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * A tool that creates private threads and verifies that PIN APIs,
 * like PIN_SafeCopy, TLS APIs, etc. work correctly in tool's threads.
 */

#include <os-apis.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <set>
#include "pin.H"
using std::cerr;
using std::dec;
using std::endl;
using std::flush;
using std::hex;
using std::ofstream;
using std::ostringstream;
using std::set;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< BOOL > KnobSafeStart(KNOB_MODE_WRITEONCE, "pintool", "safestart", "0",
                           "Ensure all internal threads are started prior to application exit");
static KNOB< string > KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "mt_tool.out", "output file");

/*!
 * Global variables.
 */

PIN_LOCK pinLock; // lock that serializes access to global vars

static ofstream out;

// Thread counters
int appThreadsStarted           = 0;
int appThreadsFinished          = 0;
int toolThreadsCreated          = 0;
int toolThreadsFinished         = 0;
volatile int toolThreadsStarted = 0;

// TLS slot for passing data to exception handlers.
TLS_KEY exceptionTlsKey;

// Argument of the main internal thread
void* rootThreadArg = (void*)0xABBA;

// Address of the DoFlush function in the application's image
volatile ADDRINT addrDoFlush = 0;

// Flags (events) that synchronize an application thread that executes DoFlush
// and tool's threads that test RTN API and CodeCache API
volatile BOOL isDoFlushCalled      = FALSE;
volatile BOOL isRtnTestCompleted   = FALSE;
volatile BOOL isCacheTestCompleted = FALSE;

volatile BOOL isCacheFlushHappened = FALSE;

// Set in PrepareForFini to signal that process is about to finish.
volatile BOOL isProcessExiting = FALSE;

// IDs of internal threads
typedef enum
{
    idRootThread = 0,
    idThreadSafeCopy,
    idThreadException,
    idThreadRtn,
    idThreadCodeCache,
    idDummyThread,
    numThreadIds
} ID_TOOL_THREAD;

// Names of internal threads
const CHAR* threadNameStr[] = {"RootThread", "ThreadSafeCopy", "ThreadException", "ThreadRtn", "ThreadCodeCache", "DummyThread"};

// Internal thread status (for debugging purposes)
typedef enum
{
    statusInit = 0,
    statusCreated,
    statusRun,
    statusTerminated,
} THREAD_STATUS;

THREAD_STATUS threadStatus[numThreadIds];
PIN_THREAD_UID threadUids[numThreadIds];

// Wait timeout in milliseconds used in PIN_SpawnInternalThread().
const UINT32 waitTimeout = 30000;

//==========================================================================
// Forward declarations
//==========================================================================
static void NotifyToolThreadExit(ID_TOOL_THREAD idToolThread);

//==========================================================================
// Utilities
//==========================================================================

/*!
 * Convert a (function) pointer to ADDRINT.
 */
template< typename PF > ADDRINT Ptr2Addrint(PF pf)
{
    union CAST
    {
        PF pf;
        ADDRINT addr;
    } cast;
    cast.pf = pf;
    return cast.addr;
}

/*!
 * Print out the error message and exit the process.
 */
static void AbortProcess(const string& msg)
{
    THREADID myTid = PIN_ThreadId();

    PIN_GetLock(&pinLock, myTid + 1);
    out << "mt_tool test aborted: " << msg << "." << endl << flush;
    PIN_ReleaseLock(&pinLock);
    PIN_WriteErrorMessage(msg.c_str(), 1002, PIN_ERR_FATAL, 0);
}

/*!
 * Print out the error message and exit the thread.
 */
static void AbortThread(const string& msg, ID_TOOL_THREAD idToolThread)
{
    THREADID myTid = PIN_ThreadId();

    PIN_GetLock(&pinLock, myTid + 1);
    out << "mt_tool thread aborted: " << msg << ". Thread name = " << threadNameStr[idToolThread] << ", tid = " << myTid << endl
        << flush;
    PIN_ReleaseLock(&pinLock);
    NotifyToolThreadExit(idToolThread);
    PIN_ExitThread(-1);
}

/*!
 * Print out the thread creation message and increase the count of threads created by tool.
 */
static void NotifyToolThreadCreated(THREADID threadId, PIN_THREAD_UID threadUid, ID_TOOL_THREAD idToolThread)
{
    threadStatus[idToolThread] = statusCreated;

    THREADID myTid = PIN_ThreadId();

    PIN_GetLock(&pinLock, myTid + 1);

    ++toolThreadsCreated;
    out << "Tool spawned a private thread. Thread name = " << threadNameStr[idToolThread] << ", tid = " << threadId << endl
        << flush;

    BOOL insertStatus = FALSE;
    if (threadUids[idToolThread] == INVALID_PIN_THREAD_UID)
    {
        threadUids[idToolThread] = threadUid;
        insertStatus             = TRUE;
    }

    PIN_ReleaseLock(&pinLock);

    if (!insertStatus)
    {
        AbortProcess("UID is not unique");
    }
}

/*!
 * Print out the thread start message and increase the count of tool's thread that started running.
 */
static void NotifyToolThreadStart(ID_TOOL_THREAD idToolThread)
{
    threadStatus[idToolThread] = statusRun;

    if (PIN_IsApplicationThread())
    {
        AbortThread("PIN_IsApplicationThread() returns TRUE for a tool's thread", idToolThread);
    }

    THREADID myTid = PIN_ThreadId();

    PIN_GetLock(&pinLock, myTid + 1);
    ++toolThreadsStarted;
    out << "Tool's thread started running, name = " << threadNameStr[idToolThread] << ", tid = " << myTid << endl << flush;
    PIN_ReleaseLock(&pinLock);
}

/*!
 * Print out the thread exit message and increase the count of finished tool's thread.
 */
static void NotifyToolThreadExit(ID_TOOL_THREAD idToolThread)
{
    threadStatus[idToolThread] = statusTerminated;

    THREADID myTid = PIN_ThreadId();

    PIN_GetLock(&pinLock, myTid + 1);
    ++toolThreadsFinished;
    out << "Tool's thread finished, name = " << threadNameStr[idToolThread] << ", tid = " << myTid << endl << flush;
    PIN_ReleaseLock(&pinLock);
}

/*!
 * Wait until isProcessExiting is set in PrepareForFini().
 */
static void WaitForProcessExit()
{
    THREADID myTid = PIN_ThreadId();

    PIN_GetLock(&pinLock, myTid + 1);
    out << "Internal thread " << myTid << " waits for process exit." << endl << flush;
    PIN_ReleaseLock(&pinLock);

    unsigned i = 0;
    while (!isProcessExiting)
    {
        PIN_Yield();

        if ((++i % 10) == 0)
        {
            PIN_Sleep(10);
        }
    }
}

/*!
 * Ensure all internal threads are started before application enters shutdown phase.
 * Should be called in application thread.
 */
static void SafeStart()
{
    if (!PIN_IsApplicationThread())
    {
        AbortProcess("Safe start should run in application thread");
    }

    // Postpone run of current application thread until all internal threads are started.
    // Note: Assume other threads are not blocked by OS at this point!
    // Note: RootThread that spawns other internal threads doesn't acquire
    // either Pin VM or Pin client lock and uses only Pin APIs that don't aquire these locks.
    for (int i = 0; i < 30; ++i)
    {
        if (toolThreadsStarted == numThreadIds)
        {
            // Success. All internal threads were started.
            THREADID myTid = PIN_ThreadId();
            PIN_GetLock(&pinLock, myTid + 1);
            out << "Safe start succeeded" << endl << flush;
            PIN_ReleaseLock(&pinLock);
            return;
        }
        // Sleep for 1 sec.
        PIN_Sleep(1000);
    }
    // Abort process if all internal threads were not started in 30 sec.
    AbortProcess("Safe start didn't complete in 30 sec.");
}

//==========================================================================
/*!
 * Exception handler for the ThreadException() procedure
 */
EXCEPT_HANDLING_RESULT ExceptionHandler(THREADID tid, EXCEPTION_INFO* pExceptInfo, PHYSICAL_CONTEXT* pPhysCtxt, VOID* arg)
{
    // Read the page address from the TLS slot. The same address is passed in <arg>.
    VOID* page = PIN_GetThreadData(exceptionTlsKey, tid);
    if (page != arg)
    {
        AbortThread("PIN_GetThreadData failed", idThreadException);
    }

    // Allow write access to the page. This should fix the problem in the thread that
    // attempts to write into this page.
    OS_RETURN_CODE res =
        OS_ProtectMemory(NATIVE_PID_CURRENT, page, getpagesize(),
                         OS_PAGE_PROTECTION_TYPE_READ | OS_PAGE_PROTECTION_TYPE_WRITE | OS_PAGE_PROTECTION_TYPE_EXECUTE);
    ASSERTX(OS_RETURN_CODE_IS_SUCCESS(res));

    return EHR_HANDLED;
}

//==========================================================================
// Internal thread procedures (ROOT_THREAD_FUNC) spawned by the tool
//==========================================================================

/*!
 * Tool's thread that verifies PIN_SafeCopy
 */
static VOID ThreadSafeCopy(VOID* arg)
{
    NotifyToolThreadStart(idThreadSafeCopy);

    char dst[16];
    char src[16];
    size_t size;

    THREADID myTid = PIN_ThreadId();

    PIN_GetLock(&pinLock, myTid + 1);
    out << "Safe copy 0 => dst" << endl << flush;
    PIN_ReleaseLock(&pinLock);

    /* TODO: Enable once Mantis 4906 is fixed.
    size = PIN_SafeCopy(dst, NULL, sizeof(dst));
    if (size != 0)
    {
        AbortThread("First PIN_SafeCopy failed", idThreadSafeCopy);
    }
*/

    PIN_GetLock(&pinLock, myTid + 1);
    out << "Safe copy src => dst" << endl << flush;
    PIN_ReleaseLock(&pinLock);

    size = PIN_SafeCopy(dst, src, sizeof(src));
    if (size != sizeof(src))
    {
        AbortThread("Second PIN_SafeCopy failed", idThreadSafeCopy);
    }

    WaitForProcessExit();
    NotifyToolThreadExit(idThreadSafeCopy);
    PIN_ExitThread(0);
}

/*!
 * Tool's thread that verifies handling of internal exceptions
 */
static VOID ThreadException(VOID* arg)
{
    NotifyToolThreadStart(idThreadException);

    THREADID myTid = PIN_ThreadId();

    // Allocate a non-writable page and
    char* page = NULL;
    OS_AllocateMemory(NATIVE_PID_CURRENT, OS_PAGE_PROTECTION_TYPE_READ | OS_PAGE_PROTECTION_TYPE_EXECUTE, getpagesize(),
                      OS_MEMORY_FLAGS_PRIVATE, (void**)&page);
    ASSERTX(page != 0);

    // To verify TLS API, we pass the page address to the exception handler in the TLS slot
    BOOL tlsStatus = PIN_SetThreadData(exceptionTlsKey, page, myTid);
    if (!tlsStatus)
    {
        AbortThread("PIN_SetThreadData failed", idThreadException);
    }

    // Attempt to write into non-writable page to cause an exception
    const char val = 0xAB;
    PIN_TryStart(myTid, ExceptionHandler, page);
    *page = val; // causes an exception which is handled by the ExceptionHandler() callback
    PIN_TryEnd(myTid);

    if (*page != val)
    {
        AbortThread("Exception handling failed", idThreadException);
    }

    WaitForProcessExit();
    NotifyToolThreadExit(idThreadException);
    PIN_ExitThread(0);
}

/*!
 * Tool's thread that verifies RTN API
 */
static VOID ThreadRtn(VOID* arg)
{
    NotifyToolThreadStart(idThreadRtn);

    // Wait until <addrDoFlush> is initialized in the ImageLoad() callback
    while (addrDoFlush == 0)
    {
        PIN_Yield();
    }

    if (addrDoFlush == ADDRINT(-1))
    {
        // There is no DoFlush routine in the application image
        NotifyToolThreadExit(idThreadRtn);
        PIN_ExitThread(0);
    }

    // Now, wait until DoFlush is called
    while (!isDoFlushCalled)
    {
        PIN_Yield();
    }

    string errMsg;

    // Look for a routine at the <addrDoFlush> address and check its name
    PIN_LockClient();

    RTN rtn = RTN_FindByAddress(addrDoFlush);
    if (!RTN_Valid(rtn))
    {
        errMsg = "RTN_FindByAddress failed";
    }
    else if (RTN_Name(rtn) != "DoFlush")
    {
        errMsg = "RTN_Name failed";
    }

    PIN_UnlockClient();

    // Continue execution of DoFlush
    isRtnTestCompleted = TRUE;

    if (!errMsg.empty())
    {
        AbortThread(errMsg, idThreadRtn);
    }

    WaitForProcessExit();
    NotifyToolThreadExit(idThreadRtn);
    PIN_ExitThread(0);
}

/*!
 * Tool's thread that verifies CodeCache API
 */
static VOID ThreadCodeCache(VOID* arg)
{
    NotifyToolThreadStart(idThreadCodeCache);

    // Wait until <addrDoFlush> is initialized in the ImageLoad() callback
    while (addrDoFlush == 0)
    {
        PIN_Yield();
    }

    if (addrDoFlush == ADDRINT(-1))
    {
        // There is no DoFlush routine in the application image
        NotifyToolThreadExit(idThreadCodeCache);
        PIN_ExitThread(0);
    }

    // Now, wait until DoFlush is called
    while (!isDoFlushCalled)
    {
        PIN_Yield();
    }

    // Flush the Code Cache
    PIN_RemoveInstrumentation();

    // Continue execution of DoFlush
    isCacheTestCompleted = TRUE;

    WaitForProcessExit();
    NotifyToolThreadExit(idThreadCodeCache);
    PIN_ExitThread(0);
}

/*!
 * Tool's thread that does nothing
 */
static VOID DummyThread(VOID* arg)
{
    NotifyToolThreadStart(idDummyThread);
    NotifyToolThreadExit(idDummyThread);
}

/*!
 * Main tool's thread. It is created in the application thread by the
 * main() tool's procedure.
 */
static VOID RootThread(VOID* arg)
{
    NotifyToolThreadStart(idRootThread);

    // Verify the argument
    if (arg != rootThreadArg)
    {
        AbortThread("Invalid thread argument", idRootThread);
    }

    THREADID myTid = PIN_ThreadId();

    THREADID threadId;
    PIN_THREAD_UID threadUid;

    // Create an internal thread and wait for exit.
    threadId = PIN_SpawnInternalThread(DummyThread, 0, 0, &threadUid);
    if (threadId == INVALID_THREADID)
    {
        if (!isProcessExiting)
        {
            // Assume that OS rejection to create thread doesn't relate to shutdown.
            AbortThread("PIN_SpawnInternalThread(DummyThread) failed", idRootThread);
        }
        else
        {
            PIN_GetLock(&pinLock, myTid + 1);
            out << "PIN_SpawnInternalThread(DummyThread) failed once process exit started" << endl << flush;
            PIN_ReleaseLock(&pinLock);
        }
    }
    else
    {
        NotifyToolThreadCreated(threadId, threadUid, idDummyThread);

        BOOL waitStatus = PIN_WaitForThreadTermination(threadUid, waitTimeout, 0); // Wait waitTimeout/1000 sec
        if (!waitStatus)
        {
            ostringstream ostr;
            ostr << "DummyThread didn't finish in " << waitTimeout / 1000 << " seconds";
            if (threadStatus[idDummyThread] >= statusRun)
            {
                AbortThread(ostr.str(), idRootThread);
            }
            else
            {
                PIN_GetLock(&pinLock, myTid + 1);
                out << ostr.str() << endl << flush;
                PIN_ReleaseLock(&pinLock);
            }
        }
    }

    // Create an internal thread that verifies PIN_SafeCopy API
    threadId = PIN_SpawnInternalThread(ThreadSafeCopy, 0, 0, &threadUid);
    if (threadId == INVALID_THREADID)
    {
        if (!isProcessExiting)
        {
            // Assume that OS rejection to create thread doesn't relate to shutdown.
            AbortThread("PIN_SpawnInternalThread(ThreadSafeCopy) failed", idRootThread);
        }
        else
        {
            PIN_GetLock(&pinLock, myTid + 1);
            out << "PIN_SpawnInternalThread(ThreadSafeCopy) failed once process exit started" << endl << flush;
            PIN_ReleaseLock(&pinLock);
        }
    }
    else
    {
        NotifyToolThreadCreated(threadId, threadUid, idThreadSafeCopy);
    }

    // Create an internal thread that verifies exception handling API
    threadId = PIN_SpawnInternalThread(ThreadException, 0, 0, &threadUid);
    if (threadId == INVALID_THREADID)
    {
        if (!isProcessExiting)
        {
            // Assume that OS rejection to create thread doesn't relate to shutdown.
            AbortThread("PIN_SpawnInternalThread(ThreadException) failed", idRootThread);
        }
        else
        {
            PIN_GetLock(&pinLock, myTid + 1);
            out << "PIN_SpawnInternalThread(ThreadException) failed once process exit started" << endl << flush;
            PIN_ReleaseLock(&pinLock);
        }
    }
    else
    {
        NotifyToolThreadCreated(threadId, threadUid, idThreadException);
    }

    // Create an internal thread that verifies RTN API
    threadId = PIN_SpawnInternalThread(ThreadRtn, 0, 0, &threadUid);
    if (threadId == INVALID_THREADID)
    {
        if (!isProcessExiting)
        {
            // Assume that OS rejection to create thread doesn't relate to shutdown.
            AbortThread("PIN_SpawnInternalThread(ThreadRtn) failed", idRootThread);
        }
        else
        {
            PIN_GetLock(&pinLock, myTid + 1);
            out << "PIN_SpawnInternalThread(ThreadRtn) failed once process exit started" << endl << flush;
            PIN_ReleaseLock(&pinLock);
        }
    }
    else
    {
        NotifyToolThreadCreated(threadId, threadUid, idThreadRtn);
    }

    // Create an internal thread that verifies CodeCache API
    threadId = PIN_SpawnInternalThread(ThreadCodeCache, 0, 0, &threadUid);
    if (threadId == INVALID_THREADID)
    {
        if (!isProcessExiting)
        {
            // Assume that OS rejection to create thread doesn't relate to shutdown.
            AbortThread("PIN_SpawnInternalThread(ThreadCodeCache) failed", idRootThread);
        }
        else
        {
            PIN_GetLock(&pinLock, myTid + 1);
            out << "PIN_SpawnInternalThread(ThreadCodeCache) failed once process exit started" << endl << flush;
            PIN_ReleaseLock(&pinLock);
        }
    }
    else
    {
        NotifyToolThreadCreated(threadId, threadUid, idThreadCodeCache);
    }

    WaitForProcessExit();
    NotifyToolThreadExit(idRootThread);
    PIN_ExitThread(0);
}

//==========================================================================
// Analysis routines
//==========================================================================
static VOID OnDoFlush()
{
    if (KnobSafeStart)
    {
        SafeStart();
    }

    // Notify tool's threads about a call to DoFlush
    isDoFlushCalled = TRUE;

    // Wait until completion of all dependent tests
    while (!(isRtnTestCompleted && isCacheTestCompleted))
    {
        PIN_Yield();
    }
}

//==========================================================================
// Instrumentation callbacks
//==========================================================================
/*!
 * RTN instrumentation routine.
 */
static VOID InstrumentRoutine(RTN rtn, VOID*)
{
    if (RTN_Name(rtn) == "DoFlush")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(OnDoFlush), IARG_END);
        RTN_Close(rtn);
    }
}

/*!
 * Trace instrumentation routine.
 */
static VOID InstrumentTrace(TRACE trace, VOID* v)
{
    const ADDRINT traceAddress = TRACE_Address(trace);
    ASSERTX(0 != traceAddress);
    if (traceAddress == Ptr2Addrint(RootThread))
    {
        AbortProcess("Pin attempts to instrument the tool's thread");
    }

    if (isCacheFlushHappened) return; // only need to record traces until the codecache flush

    static set< ADDRINT > addresses;
    if (isCacheTestCompleted)
    {
        if (addresses.end() != addresses.find(traceAddress))
        {
            ASSERTX(!isCacheFlushHappened);
            isCacheFlushHappened = TRUE;

            THREADID myTid = PIN_ThreadId();
            PIN_GetLock(&pinLock, myTid + 1);
            out << "Re-jitting TRACE at address 0x" << hex << traceAddress << dec << ", assuming codecache flush." << endl;
            PIN_ReleaseLock(&pinLock);
            return;
        }
    }
    addresses.insert(traceAddress);
}

/*!
 * Image load callback.
 */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        ADDRINT addr;
        RTN rtn = RTN_FindByName(img, "DoFlush");
        if (RTN_Valid(rtn))
        {
            addr = RTN_Address(rtn);
        }
        else
        {
            addr = ADDRINT(-1);
        }
        addrDoFlush = addr;
    }
}

/*!
 * Thread start/exit callbacks in application's  threads.
 */
static VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (!PIN_IsApplicationThread())
    {
        AbortProcess("PIN_IsApplicationThread() returns FALSE for an application's thread");
    }

    PIN_GetLock(&pinLock, threadid + 1);
    ++appThreadsStarted;
    out << "Application's thread started running, tid = " << threadid << endl << flush;
    PIN_ReleaseLock(&pinLock);
}

static VOID ThreadFini(THREADID threadid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    PIN_GetLock(&pinLock, threadid + 1);
    ++appThreadsFinished;
    out << "Application's thread finished, tid = " << threadid << endl << flush;
    PIN_ReleaseLock(&pinLock);
}

/*!
 * Process exit callback (unlocked).
 */
static VOID PrepareForFini(VOID* v)
{
    if (!PIN_IsApplicationThread())
    {
        AbortProcess("Process is terminated by an internal thread");
    }

    BOOL waitStatus;
    INT32 threadExitCode;
    BOOL threadExitStatus = TRUE;

    // Signal that process is about to finish.
    isProcessExiting = TRUE;

    THREADID myTid = PIN_ThreadId();

    // Wait until all internal threads exit
    for (int i = 0; i < numThreadIds; ++i)
    {
        if (threadUids[i] == INVALID_PIN_THREAD_UID)
        {
            // The thread was not created. Already reported in log.
            continue;
        }
        waitStatus = PIN_WaitForThreadTermination(threadUids[i], waitTimeout, &threadExitCode); // Wait waitTimeout/1000 sec
        if (!waitStatus)
        {
            PIN_GetLock(&pinLock, myTid + 1);
            out << threadNameStr[i] << " didn't finish in " << waitTimeout / 1000 << " seconds" << endl << flush;
            PIN_ReleaseLock(&pinLock);

            if (threadStatus[i] >= statusRun)
            {
                // Report error only if the thread is already started in the tool.
                // Since this point OS cannot terminate it without notification or infinitely suspend it.
                threadExitStatus = FALSE;
            }
        }
        else if (threadExitCode != 0)
        {
            threadExitStatus = FALSE;
        }
    }

    // If any thread exited abnormally, abort the process
    if (!threadExitStatus)
    {
        AbortProcess("At least one of the tool's threads exited abnormally");
    }

    PIN_GetLock(&pinLock, myTid + 1);
    out << "mt_tool test: All tool's threads finished successfully." << endl << flush;
    PIN_ReleaseLock(&pinLock);
}

/*!
 * Process exit callback (locked).
 */
static VOID Fini(INT32 code, VOID* v)
{
    out << "Number of application's threads started: " << appThreadsStarted << endl << flush;
    out << "Number of application's threads finished: " << appThreadsFinished << endl << flush;
    out << "Number of tool's threads created: " << toolThreadsCreated << endl << flush;
    out << "Number of tool's threads started: " << toolThreadsStarted << endl << flush;
    out << "Number of tool's threads finished: " << toolThreadsFinished << endl << flush;
    if (toolThreadsCreated != toolThreadsStarted)
    {
        out << "toolThreadsCreated: " << toolThreadsCreated << ", toolThreadsStarted: " << toolThreadsStarted << endl << flush;
        if (KnobSafeStart)
        {
            // We expect all created internal threads are started if safe start is applied.
            // This state means that safe start was not applied.
            AbortProcess("Safe start was not applied");
        }
        // If threads safe start is not applied it is OK to expect some internal threads
        // to remain not started since OS may still block them when process exit happens.
    }
    if (toolThreadsStarted != toolThreadsFinished)
    {
        out << "toolThreadsStarted: " << toolThreadsStarted << ", toolThreadsFinished: " << toolThreadsFinished << endl << flush;
        PIN_ExitProcess(1);
    }

    if (isCacheTestCompleted && !isCacheFlushHappened)
    {
        out << "Code Cache flush did not happen" << endl << flush;
        PIN_ExitProcess(1);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool tests multithreaded tools on multithreaded apps." << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/*!
 * The main procedure of the tool.
 */
int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv)) return Usage();

    out.open(KnobOutput.Value().c_str());

    PIN_InitLock(&pinLock);

    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_AddPrepareForFiniFunction(PrepareForFini, 0);
    TRACE_AddInstrumentFunction(InstrumentTrace, 0);
    IMG_AddInstrumentFunction(ImageLoad, 0);
    RTN_AddInstrumentFunction(InstrumentRoutine, 0);

    // In order to verify TLS API in internal threads we create a TLS slot for
    // passing data to exception handlers.
    exceptionTlsKey = PIN_CreateThreadDataKey(0);
    if (exceptionTlsKey == -1)
    {
        AbortProcess("PIN_CreateThreadDataKey failed");
    }

    for (int i = 0; i < numThreadIds; ++i)
    {
        threadStatus[i] = statusInit;
        threadUids[i]   = INVALID_PIN_THREAD_UID;
    }

    // Spawn the main internal thread. When this thread starts it spawns all other internal threads.
    PIN_THREAD_UID rootThreadUid;
    THREADID rootThreadId = PIN_SpawnInternalThread(RootThread, rootThreadArg, 0, &rootThreadUid);
    if (rootThreadId == INVALID_THREADID)
    {
        AbortProcess("PIN_SpawnInternalThread(RootThread) failed");
    }
    NotifyToolThreadCreated(rootThreadId, rootThreadUid, idRootThread);

    // Never returns
    PIN_StartProgram();
    return 0;
}

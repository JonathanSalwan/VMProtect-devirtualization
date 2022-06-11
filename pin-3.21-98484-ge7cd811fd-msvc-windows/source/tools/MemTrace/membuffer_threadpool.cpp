/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Sample buffering tool using Pin buffering API, with internal-tool threads that processes
 * the buffers.
 *
 * Similar to membuffer_simple,  a memory trace (Ip of memory accessing instruction and address
 * of memory access - see struct MEMREF) is collected by inserting Pin buffering API.
 * This tool uses  multiple buffers and internal-tool threads to enable the application code to continue
 * to run (and fill buffers), while full buffers are being processed by the internal-tool threads.
 * It is similar to memtrace_threadpool in this sense - but the multiple buffers that are available for filling
 * are kept on the free buffers list of the application thread that used the PIN_AllocateBuffer to allocate
 * them.
 *
 * Each application thread allocates the specified number of buffers (see PIN_AllocateBuffer).
 * The Pin buffering API requires that each application thread can only use the buffers it
 * allocated as buffers to be filled by Pin, but any internal-tool thread can process any buffer.
 * So the internal-processing threads can be used as a thread-pool.
 * This leads to the following model:
 * Buffers that are available to be used by the application thread to be filled by Pin are kept on
 * a free buffers list associated with the application thread.
 * When a buffer becomes full, Pin calls the BufferFull callback function. The BufferFull function uses
 * its thread id parameter to identify which application thread this buffer belongs to, and then
 * calls that threads EnqueueFullAndGetNextToFill function.
 * The EnqueueFullAndGetNextToFill function puts the buffer, along with the identifier of which app-thread
 * it belongs to on a global full buffers list, and then signals that a(nother) full buffer is on the
 * list. After this the EnqueueFullAndGetNextToFill function waits on the free buffers list it's application
 * thread, when this free list is signaled, the EnqueueFullAndGetNextToFill takes a buffer from there
 * and returns it to the BufferFull function, which returns it to Pin as the next buffer to fill.
 * The internal-tool threads all wait on the full buffers list. When that list is signaled, one of them
 * wakes up, takes a buffer from the list, processes it, and then puts it on the free buffers list
 * of the application thread that owns the buffer.
 *
 *
 * While the Pin APIs used here are OS generic, the tool also uses windows counting semaphores to
 * manage the buffer lists - so it is windows specific
 */

#include <cstdio>
#include <set>
#include <list>
#include "pin.H"

namespace WIND
{
#include <windows.h>
}
using WIND::DWORD;

using std::list;
using std::set;
using std::string;
/*
 * Knobs for tool
 */

KNOB< BOOL > KnobProcessBuffer(KNOB_MODE_WRITEONCE, "pintool", "process_buffs", "1", "process the filled buffers");
KNOB< UINT32 > KnobNumProcessingThreads(KNOB_MODE_OVERWRITE, "pintool", "num_processing_threads", "3",
                                        "number of processing threads");
// 256*4096=1048576 - same size buffer in memtrace_simple, membuffer_simple, memtrace_threadpool and membuffer_threadpool
KNOB< UINT32 > KnobNumPagesInBuffer(KNOB_MODE_WRITEONCE, "pintool", "num_pages_in_buffer", "256", "number of pages in buffer");
KNOB< UINT32 > KnobNumBuffersPerAppThread(KNOB_MODE_WRITEONCE, "pintool", "num_buffers_per_app_thread", "3",
                                          "number of buffers per thread");
KNOB< BOOL > KnobStatistics(KNOB_MODE_WRITEONCE, "pintool", "statistics", "0", "gather statistics");
KNOB< BOOL > KnobLiteStatistics(KNOB_MODE_WRITEONCE, "pintool", "lite_statistics", "0", "gather lite statistics");
KNOB< string > KnobStatisticsOutputFile(KNOB_MODE_WRITEONCE, "pintool", "stat_file", "membuffer_threadpool_stats.out",
                                        "output file");
extern "C" UINT64 ReadProcessorCycleCounter();

/* Struct of memory references recorded in buffers.
 */
struct MEMREF
{
    ADDRINT pc;
    ADDRINT ea;
};

// the Pin TLS slot that an application-thread will use to hold the APP_THREAD_REPRESENTITVE
// object that it owns
TLS_KEY appThreadRepresentitiveKey;

// Set of UIDs of all internal-tool threads
// We use std::set to verify that each thread has a unique UID
set< PIN_THREAD_UID > uidSet;

// The buffer ID returned by the one call to PIN_DefineTraceBuffer
BUFFER_ID bufId;

#include "threadpool_statistics.h"

class BUFFER_LIST_MANAGER;

// all full buffers are placed on this list by the app threads.
// the internal-tool threads pick them up from here,
// process them, and put them on the owning app thread's
// free list
BUFFER_LIST_MANAGER* fullBuffersListManager = NULL;

/*
 * APP_THREAD_REPRESENTITVE
 * Each application thread, creates an object of this class and saves it in it's Pin TLS
 * slot (appThreadRepresentitiveKey).
 * This object is used when the BufferFull function is called. It provides the functionality
 * of:
 * 1) Managing the buffers allocated (by Pin) by this thread. It uses it's BUFFER_LIST_MANAGER
 *    _freeBufferListManager to do this.
 * 2) Enquening a full buffer on the global full buffers list (fullBuffersListManager) so it
 *    will be processed by one of the internal-tool buffer processing threads.
 * 3) If there is no internal-tool buffer processing thread running yet
 *    then the ProcessBuffer is used to process the buffer by the application
 *    thread. It cannot wait for processing thread to start running
 *    because this may cause deadlock - because this app thread may be holding some OS
 *    resource that the processing thread needs in order to start running - e.g. the LoaderLock
 */
class APP_THREAD_REPRESENTITVE
{
  public:
    APP_THREAD_REPRESENTITVE(THREADID tid);
    ~APP_THREAD_REPRESENTITVE();

    // Called from the BufferFull callback
    VOID* EnqueueFullAndGetNextToFill(VOID* buf, UINT64 numElements);

    // Called from the ThreadFini callback, to know when all the buffers of this app thread
    // have been processed
    BOOL AllBuffersProcessed();

    APP_THREAD_STATISTICS* Statistics() { return (&_appThreadStatistics); }
    BUFFER_LIST_MANAGER& FreeBufferListManager() { return *_freeBufferListManager; }

  private:
    // the buffers of this thread are placed on this list when they are available for filling
    BUFFER_LIST_MANAGER* _freeBufferListManager;
    THREADID _myTid;
    UINT32 _numBuffersAllocated;
    VOID* _currentBuf;

    APP_THREAD_STATISTICS _appThreadStatistics;
};

/*
 * BUFFER_LIST_MANAGER
 * This class implements buffer list management, both for the global fullBuffers list
 * and for the per-app-thread bufferBuffersList
 */
class BUFFER_LIST_MANAGER
{
  public:
    BUFFER_LIST_MANAGER(BOOL notifyExitRequired = FALSE);
    ~BUFFER_LIST_MANAGER();

    BOOL PutBufferOnList(VOID* buf, UINT64 numElements,
                         /* the thread that owns the buffer */
                         APP_THREAD_REPRESENTITVE* appThreadRepresentitive,
                         /* thread Id of the thread making the call */
                         THREADID tid);
    VOID* GetBufferFromList(UINT64* numElements,
                            /* the thread that owns the buffer */
                            APP_THREAD_REPRESENTITVE** appThreadRepresentitive,
                            /* thread Id of the thread making the call */
                            THREADID tid);
    VOID NotifyExit();
    BUFFER_LIST_STATISTICS* Statistics() { return &_bufferListStatistics; }

  private:
    // structure of an element of the buffer list
    struct BUFFER_LIST_ELEMENT
    {
        VOID* buf;
        UINT64 numElements;
        // the application thread that puts this buffer on the list
        APP_THREAD_REPRESENTITVE* appThreadRepresentitive;
    };

    class SCOPED_LOCK
    {
        PIN_LOCK* _lock;

      public:
        SCOPED_LOCK(PIN_LOCK* lock, THREADID tid) : _lock(lock)
        {
            ASSERTX(_lock != NULL);
            PIN_GetLock(_lock, tid + 1);
        }
        ~SCOPED_LOCK() { PIN_ReleaseLock(_lock); }
    };

    WIND::HANDLE _exitEvent;
    WIND::HANDLE _bufferSem;
    PIN_LOCK _bufferListLock;
    list< BUFFER_LIST_ELEMENT > _bufferList;

    BUFFER_LIST_STATISTICS _bufferListStatistics;
};

/*
 * Returns single instance of global buffer list manager that supports synchronization
 * and notified on process exit
 */
BUFFER_LIST_MANAGER* GetFullBuffersListManager()
{
    static BUFFER_LIST_MANAGER buffersListManager(TRUE);
    return &buffersListManager;
}

VOID ProcessBuffer(VOID* buf, UINT64 numElements, APP_THREAD_REPRESENTITVE* associatedAppThread)
{
    if (!KnobProcessBuffer)
    {
        return;
    }

    if (KnobStatistics)
    {
        associatedAppThread->Statistics()->StartCyclesProcessingBuffer();
    }

    struct MEMREF* memref      = reinterpret_cast< struct MEMREF* >(buf);
    struct MEMREF* firstMemref = memref;
    for (UINT64 i = 0; i < numElements; i++, memref++)
    {
        firstMemref->pc += memref->pc + memref->ea;
    }

    associatedAppThread->Statistics()->AddNumElementsProcessed((UINT32)numElements);
    if (KnobStatistics)
    {
        associatedAppThread->Statistics()->UpdateCyclesProcessingBuffer();
    }
}

/*********** APP_THREAD_REPRESENTITVE implementation *******/

APP_THREAD_REPRESENTITVE::APP_THREAD_REPRESENTITVE(THREADID tid) : _myTid(tid), _numBuffersAllocated(0), _currentBuf(NULL)
{
    _freeBufferListManager = new (BUFFER_LIST_MANAGER);
}

APP_THREAD_REPRESENTITVE::~APP_THREAD_REPRESENTITVE() { delete _freeBufferListManager; }

VOID* APP_THREAD_REPRESENTITVE::EnqueueFullAndGetNextToFill(VOID* fullBuf, UINT64 numElements)
{
    _appThreadStatistics.IncrementNumBuffersFilled();

    // under some conditions the buffer is processed in this app thread
    if ((fullBuffersListManager == NULL) // cannot wait for processing thread to start running
                                         // this may cause deadlock - because this app thread
                                         // may be holding some OS resource that the processing
                                         // needs to obtain in order to start - e.g. the LoaderLock
        // heuristic - no available free buffer, so process by this app thread.

        // Otherwise put the fullBuf on the full buffers list, one the internal-tool processing
        // threads will pick it from there, process it, and then put it on this app-thread's
        // free buffer list
        || !fullBuffersListManager->PutBufferOnList(fullBuf, numElements, this, _myTid)
        // Full buffers manager may not take the buffer if process exit is started.
    )
    { // process buffer in this app thread
        _appThreadStatistics.IncrementNumBuffersProcessedInAppThread();
        ProcessBuffer(fullBuf, numElements, this);
        _currentBuf = fullBuf;
        return _currentBuf;
    }

    if (_numBuffersAllocated == 0)
    {
        // now allocate the rest of the KnobNumBuffersPerAppThread buffers to be used
        // only one buffer definition is used - it is identified by bufId
        // that was returned by the one call to PIN_DefineTraceBuffer
        for (; _numBuffersAllocated < KnobNumBuffersPerAppThread - 1; _numBuffersAllocated++)
        {
            VOID* buf = PIN_AllocateBuffer(bufId);
            _freeBufferListManager->PutBufferOnList(buf, 0, this, _myTid);
            printf("    Allocated buffer %p for thread %d\n", buf, _myTid);
            fflush(stdout);
        }
    }

    // provide Pin with the next buffer to fill.
    // It is always taken from the free buffers list of this app thread.
    // If the list is empty then this app thread will be blocked until one
    // is placed there (by one of the tool-internal buffer processing threads).
    UINT64 numElementsDummy;
    APP_THREAD_REPRESENTITVE* appThreadRepresentitiveDummy;
    _currentBuf = _freeBufferListManager->GetBufferFromList(&numElementsDummy, &appThreadRepresentitiveDummy, _myTid);
    ASSERTX(_currentBuf != NULL);
    ASSERTX(appThreadRepresentitiveDummy == this);
    return _currentBuf;
}

BOOL APP_THREAD_REPRESENTITVE::AllBuffersProcessed()
{
    if (_numBuffersAllocated == 0)
    {
        return TRUE;
    }

    ASSERTX(_currentBuf != NULL);

    // Reclaim allocated buffers
    for (; _numBuffersAllocated > 0; _numBuffersAllocated--)
    {
        UINT64 numElementsDummy;
        APP_THREAD_REPRESENTITVE* appThreadRepresentitiveDummy = NULL;
        VOID* buf = _freeBufferListManager->GetBufferFromList(&numElementsDummy, &appThreadRepresentitiveDummy, _myTid);
        ASSERTX(buf != NULL);
        ASSERTX(appThreadRepresentitiveDummy == this);
        PIN_DeallocateBuffer(bufId, buf);
        printf("    Deallocated buffer %p for thread %d\n", buf, _myTid);
        fflush(stdout);
    }
    PIN_DeallocateBuffer(bufId, _currentBuf);
    printf("    Deallocated buffer %p for thread %d\n", _currentBuf, _myTid);
    fflush(stdout);
    return TRUE;
}

/*********** BUFFER_LIST_MANAGER implementation *******/

BUFFER_LIST_MANAGER::BUFFER_LIST_MANAGER(BOOL notifyExitRequired) : _exitEvent(NULL)
{
    PIN_InitLock(&_bufferListLock);
    _bufferSem = WIND::CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
    ASSERTX(_bufferSem != NULL);
    if (notifyExitRequired)
    {
        // Create manually reset event in non-signaled state.
        _exitEvent = WIND::CreateEvent(NULL, TRUE, FALSE, NULL);
        ASSERTX(_exitEvent != NULL);
    }
}

BUFFER_LIST_MANAGER::~BUFFER_LIST_MANAGER()
{
    if (_exitEvent != NULL)
    {
        WIND::CloseHandle(_exitEvent);
    }
    WIND::CloseHandle(_bufferSem);
}

BOOL BUFFER_LIST_MANAGER::PutBufferOnList(VOID* buf, UINT64 numElements,
                                          /* the thread that owns the buffer */
                                          APP_THREAD_REPRESENTITVE* appThreadRepresentitive,
                                          /* thread Id of the thread making the call */
                                          THREADID tid)
{
    if ((_exitEvent != NULL) && (WIND::WaitForSingleObject(_exitEvent, 0) == WAIT_OBJECT_0))
    {
        // Exit event signaled. Do not add new buffers.
        return FALSE;
    }

    BUFFER_LIST_ELEMENT bufferListElement;

    bufferListElement.buf                     = buf;
    bufferListElement.numElements             = numElements;
    bufferListElement.appThreadRepresentitive = appThreadRepresentitive;

    {
        SCOPED_LOCK lock(&_bufferListLock, tid);
        _bufferList.push_back(bufferListElement);
    }

    WIND::ReleaseSemaphore(_bufferSem, 1, NULL);

    return TRUE;
}

VOID* BUFFER_LIST_MANAGER::GetBufferFromList(UINT64* numElements,
                                             /* the thread that owns the buffer */
                                             APP_THREAD_REPRESENTITVE** appThreadRepresentitive,
                                             /* thread Id of the thread making the call */
                                             THREADID tid)
{
    if (KnobStatistics)
    {
        if (_bufferList.empty())
        {
            _bufferListStatistics.IncrementNumTimesWaited();
        }
        _bufferListStatistics.StartCyclesWaitingForBuffer();
    }

    WIND::DWORD status;
    if (_exitEvent != NULL)
    {
        // Process buffers even after exit notification until the list is empty.
        WIND::HANDLE handles[2] = {_bufferSem, _exitEvent};
        status                  = WIND::WaitForMultipleObjects(2, handles, FALSE, INFINITE);
        if (status == (WAIT_OBJECT_0 + 1))
        {
            // Process exit flow started and there is no pending buffers to process.
            return NULL;
        }
    }
    else
    {
        status = WIND::WaitForSingleObject(_bufferSem, INFINITE);
    }
    // Otherwise wait should exit due to semaphore.
    if (status != WAIT_OBJECT_0)
    {
        printf("  WAIT returned %d, last error %x\n", status, WIND::GetLastError());
        fflush(stdout);
        ASSERTX(status == WAIT_OBJECT_0);
    }

    if (KnobStatistics)
    {
        _bufferListStatistics.UpdateCyclesWaitingForBuffer();
    }

    {
        SCOPED_LOCK lock(&_bufferListLock, tid);
        ASSERTX(!_bufferList.empty());
        const BUFFER_LIST_ELEMENT& bufferListElement = (_bufferList.front());
        VOID* buf                                    = bufferListElement.buf;
        *numElements                                 = bufferListElement.numElements;
        *appThreadRepresentitive                     = bufferListElement.appThreadRepresentitive;
        _bufferList.pop_front();
        return buf;
    }
}

VOID BUFFER_LIST_MANAGER::NotifyExit()
{
    if (_exitEvent != NULL)
    {
        WIND::SetEvent(_exitEvent);
    }
}

/*********** BUFFER_LIST_MANAGER implementation END *******/

/*
 * Trace instrumentation routine invoked by Pin when jitting a trace
 * Insert code to write data to a thread-specific buffer for instructions
 * that access memory.
 */
VOID Trace(TRACE trace, VOID* v)
{
    // Insert a call to record the effective address.
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            UINT32 memOperands = INS_MemoryOperandCount(ins);

            // Iterate over each memory operand of the instruction.
            for (UINT32 memOp = 0; memOp < memOperands; memOp++)
            {
                INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId, IARG_INST_PTR, offsetof(struct MEMREF, pc), IARG_MEMORYOP_EA,
                                     memOp, offsetof(struct MEMREF, ea), IARG_END);
            }
        }
    }
}

/**************************************************************************
 *
 *  Callback Routines
 *
 **************************************************************************/

/*!
 * Called when a buffer fills up, or the thread exits, so the buffer can be processed
 * Called in the context of the application thread
 * @param[in] id        buffer handle
 * @param[in] tid       id of owning thread
 * @param[in] ctxt      application context
 * @param[in] buf       actual pointer to buffer
 * @param[in] numElements   number of records
 * @param[in] v         callback value
 * @return  A pointer to the buffer to resume filling.
 */
VOID* BufferFull(BUFFER_ID id, THREADID tid, const CONTEXT* ctxt, VOID* buf, UINT64 numElements, VOID* v)
{
    struct MEMREF* reference = (struct MEMREF*)buf;

    APP_THREAD_REPRESENTITVE* appThreadRepresentitive =
        static_cast< APP_THREAD_REPRESENTITVE* >(PIN_GetThreadData(appThreadRepresentitiveKey, tid));
    ASSERTX(appThreadRepresentitive != NULL);

    printf("AppThread tid %d  GotBuffer %p\n", tid, buf);
    fflush(stdout);
    VOID* nextBuffToFill = appThreadRepresentitive->EnqueueFullAndGetNextToFill(buf, numElements);
    printf("AppThread tid %d  NextToFill %p\n", tid, nextBuffToFill);
    fflush(stdout);
    return (nextBuffToFill);
}

VOID ThreadStart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    // There is a new APP_THREAD_REPRESENTITVE for every thread.
    APP_THREAD_REPRESENTITVE* appThreadRepresentitive = new APP_THREAD_REPRESENTITVE(tid);

    printf("Started thread %d\n", tid);
    fflush(stdout);

    // A thread will need to look up its APP_THREAD_REPRESENTITVE, so save pointer in TLS
    PIN_SetThreadData(appThreadRepresentitiveKey, appThreadRepresentitive, tid);
}

VOID ThreadFini(THREADID tid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    APP_THREAD_REPRESENTITVE* appThreadRepresentitive =
        static_cast< APP_THREAD_REPRESENTITVE* >(PIN_GetThreadData(appThreadRepresentitiveKey, tid));
    ASSERTX(appThreadRepresentitive != NULL);

    printf("Finished thread %d\n", tid);
    fflush(stdout);

    // wait for all my buffers to be processed
    BOOL ok = appThreadRepresentitive->AllBuffersProcessed();
    ASSERTX(ok);

    appThreadRepresentitive->Statistics()->DumpNumBuffersFilled();
    appThreadRepresentitive->Statistics()->IncorporateBufferStatistics(
        appThreadRepresentitive->FreeBufferListManager().Statistics());
    overallStatistics.AccumulateAppThreadStatistics(appThreadRepresentitive->Statistics(), TRUE);
    if (KnobStatistics)
    {
        appThreadRepresentitive->Statistics()->Dump();
    }

    delete appThreadRepresentitive;

    PIN_SetThreadData(appThreadRepresentitiveKey, 0, tid);
}

/*!
 * Process exit callback (unlocked).
 */
static VOID PrepareForFini(VOID* v)
{
    printf("PrepareForFini\n");
    fflush(stdout);

    if (fullBuffersListManager == NULL)
    {
        printf("Internal threads were not yet started.\n");
        fflush(stdout);
        // Ensure that global buffer manager exists on process exit,
        // otherwise it could be created later without exit notification.
        fullBuffersListManager = GetFullBuffersListManager();
    }
    // Notify full buffers manager of application exit.
    // It will cause all internal threads to exit once they finish processing of pending buffers.
    fullBuffersListManager->NotifyExit();

    // Wait until all internal threads exit
    for (set< PIN_THREAD_UID >::iterator it = uidSet.begin(); it != uidSet.end(); ++it)
    {
        printf("Waiting for exit of thread uid %d.\n", *it);
        fflush(stdout);
        INT32 threadExitCode;
        BOOL waitStatus = PIN_WaitForThreadTermination(*it, PIN_INFINITE_TIMEOUT, &threadExitCode);
        if (!waitStatus)
        {
            fprintf(stderr, "PIN_WaitForThreadTermination(secondary thread) failed");
        }
    }
}

static VOID Fini(INT32 code, VOID* v)
{
    overallStatistics.DumpNumBuffersFilled();
    if (fullBuffersListManager != NULL)
    {
        overallStatistics.IncorporateBufferStatistics(fullBuffersListManager->Statistics(), TRUE);
    }
    if (KnobStatistics)
    {
        overallStatistics.Dump();
    }
}

/*!
 * Record the thread's uid
 */
static VOID RecordToolThreadCreated(PIN_THREAD_UID threadUid)
{
    BOOL insertStatus;
    insertStatus = (uidSet.insert(threadUid)).second;
    if (!insertStatus)
    {
        fprintf(stderr, "UID is not unique");
        exit(-1);
    }
}

/*
  Buffer Processing Thread's routine
*/
static VOID BufferProcessingThread(VOID* arg)
{
    fullBuffersListManager = GetFullBuffersListManager();
    THREADID myThreadId    = PIN_ThreadId();

    for (;;)
    {
        UINT64 numElements;
        APP_THREAD_REPRESENTITVE* appThreadRepresentitive;
        printf("BufferProcessingThread tid %d  GetBufferFromList\n", myThreadId);
        fflush(stdout);
        VOID* buf = fullBuffersListManager->GetBufferFromList(&numElements, &appThreadRepresentitive, myThreadId);
        if (buf == NULL)
        {
            printf("BufferProcessingThread tid %d is exiting\n", myThreadId);
            PIN_ExitThread(0); // Doesn't return
            return;
        }
        printf("BufferProcessingThread tid %d  ProcessBuffer %p\n", myThreadId, buf);
        fflush(stdout);
        ProcessBuffer(buf, numElements, appThreadRepresentitive);
        printf("BufferProcessingThread tid %d  return buffer %p to appThreadRepresentitive %p\n", myThreadId, buf,
               appThreadRepresentitive);
        fflush(stdout);
        appThreadRepresentitive->FreeBufferListManager().PutBufferOnList(buf, 0, appThreadRepresentitive, myThreadId);
        //printf ("BufferProcessingThread tid %d appThreadRepresentitive %p now has %d buffers on it free list\n",
        //        myThreadId, appThreadRepresentitive, appThreadRepresentitive->FreeBufferListManager()->NumBuffersOnList());
    }
}

/*!
 *  Print out help message.
 */

INT32 Usage()
{
    printf("This tool demonstrates the advanced use of the buffering API in conjunction \nwith internal-tool threads\n");
    printf("The following command line options are available:\n");
    printf("-num_buffers_per_app_thread <num>  :number of buffers to allocate per application thread,        default   3\n");
    printf("-num_pages_in_buffer <num>         :number of (4096byte) pages allocated in each buffer,         default 256\n");
    printf("-process_buffs <0 or 1>            :specify 0 to disable processing of the buffers,              default   1\n");
    printf("-num_processing_threads <num>      :number of internal-tool buffer processing threads to create, default   3\n");
    printf("-lite_statistics  <0 or 1>         :specify 1 to enable lite statistics gathering,               default   0\n");
    printf("-heavy_statistics <0 or 1>         :specify 1 to enable heavy statistics gathering,              default   0\n");

    return -1;
}

/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments,
 *                              including pin -t <toolname> -- ...
 */
int main(int argc, char* argv[])
{
    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    // Define the buffer type to be used
    // The first buffer of this definition is implicitly allocated to each application thread
    // by Pin when the application thread starts. The rest of the buffers are explicitly
    // allocated by the application thread when it has determined that it has an associated
    // internal-tool thread that has started running - see call to PIN_AllocateBuffer
    bufId = PIN_DefineTraceBuffer(sizeof(struct MEMREF), KnobNumPagesInBuffer, BufferFull, 0);

    if (bufId == BUFFER_ID_INVALID)
    {
        return 1;
    }

    // Initialize Pin TLS slot used by the application threads to store and
    // retrieve the APP_THREAD_REPRESENTITVE object that they own
    appThreadRepresentitiveKey = PIN_CreateThreadDataKey(0);

    // add an instrumentation function
    TRACE_AddInstrumentFunction(Trace, 0);

    // add callbacks
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_AddPrepareForFiniFunction(PrepareForFini, 0);

    /* It is safe to create internal threads in the tool's main procedure and spawn new
     * internal threads from existing ones. All other places, like Pin callbacks and
     * analysis routines in application threads, are not safe for creating internal threads.
    */
    // Spawn the tool's internal threads.
    for (int i = 0; i < KnobNumProcessingThreads; i++)
    {
        PIN_THREAD_UID threadUid;
        THREADID threadId = PIN_SpawnInternalThread(BufferProcessingThread, NULL, 0, &threadUid);
        if (threadId == INVALID_THREADID)
        {
            fprintf(stderr, "PIN_SpawnInternalThread(BufferProcessingThread) failed");
            exit(-1);
        }
        printf("created internal-tool BufferProcessingThread\n");
        fflush(stdout);
        RecordToolThreadCreated(threadUid);
    }

    printf("buffer size in bytes 0x%x\n", KnobNumPagesInBuffer * 4096);
    overallStatistics.Init();
    fflush(stdout);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

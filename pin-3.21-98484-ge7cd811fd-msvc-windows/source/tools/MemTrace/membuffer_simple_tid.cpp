/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *
 * A memory trace (Ip of memory accessing instruction and address of memory access - see
 * struct MEMREF) is collected by inserting Pin buffering API code into the application code,
 * via calls to INS_InsertFillBuffer. This analysis code writes a MEMREF into the
 * buffer being filled, and calls the registered BufferFull function (see call to
 * PIN_DefineTraceBuffer which defines the buffer and registers the BufferFull function)
 * when the buffer becomes full.
 * The BufferFull function processes the buffer and returns it to Pin to be filled again.
 *
 * Each application thread has it's own buffer - so multiple application threads do NOT
 * block each other on buffer accesses
 *
 * This tool is similar to membuffer_simple, but it checks to see that the BufferFull function
 * can be executed simultaneously by different threads
 * It must be run with the thread2 application
 *
 */

#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include "pin.H"

/*
 * Knobs for tool
 */

KNOB< BOOL > KnobProcessBuffer(KNOB_MODE_WRITEONCE, "pintool", "process_buffs", "1", "process the filled buffers");
// 256*4096=1048576 - same size buffer in memtrace_simple, membuffer_simple, membuffer_multi
KNOB< UINT32 > KnobNumPagesInBuffer(KNOB_MODE_WRITEONCE, "pintool", "num_pages_in_buffer", "256", "number of pages in buffer");

/* Struct of memory reference written to the buffer
 */
struct MEMREF
{
    ADDRINT pc;
    ADDRINT ea;
    THREADID tid;
};

// The buffer ID returned by the one call to PIN_DefineTraceBuffer
BUFFER_ID bufId;

// the Pin TLS slot that an application-thread will use to hold the APP_THREAD_REPRESENTITVE
// object that it owns
TLS_KEY appThreadRepresentitiveKey;

UINT32 totalBuffersFilled                             = 0;
UINT64 totalElementsProcessed                         = 0;
volatile int maxThreadsSimultaneosulyInProcessBuffer  = 0;
volatile int numThreadsSimultaneosulyInProcessBuffer  = 0;
const int numThreadsInApp                             = 16;
volatile int numThreadsCalledWaitForAllThreadsStarted = 0;

/*
 *
 * APP_THREAD_REPRESENTITVE
 *
 * Each application thread, creates an object of this class and saves it in it's Pin TLS
 * slot (appThreadRepresentitiveKey).
 */
class APP_THREAD_REPRESENTITVE
{
  public:
    APP_THREAD_REPRESENTITVE(THREADID tid);
    ~APP_THREAD_REPRESENTITVE();

    VOID ProcessBuffer(VOID* buf, UINT64 numElements, THREADID tid);
    UINT32 NumBuffersFilled() { return _numBuffersFilled; }

    UINT32 NumElementsProcessed() { return _numElementsProcessed; }

  private:
    UINT32 _numBuffersFilled;
    UINT32 _numElementsProcessed;
};

APP_THREAD_REPRESENTITVE::APP_THREAD_REPRESENTITVE(THREADID tid)
{
    _numBuffersFilled     = 0;
    _numElementsProcessed = 0;
}

APP_THREAD_REPRESENTITVE::~APP_THREAD_REPRESENTITVE() {}

PIN_LOCK lock1;
VOID APP_THREAD_REPRESENTITVE::ProcessBuffer(VOID* buf, UINT64 numElements, THREADID tid)
{
    _numBuffersFilled++;

    PIN_GetLock(&lock1, tid + 1);
    numThreadsSimultaneosulyInProcessBuffer++;
    if (numThreadsSimultaneosulyInProcessBuffer > maxThreadsSimultaneosulyInProcessBuffer)
    {
        maxThreadsSimultaneosulyInProcessBuffer = numThreadsSimultaneosulyInProcessBuffer;
    }
    PIN_ReleaseLock(&lock1);

    if (numThreadsCalledWaitForAllThreadsStarted == numThreadsInApp && maxThreadsSimultaneosulyInProcessBuffer < 2)
    { // it is now safe to wait to see at least two threads in this function.
        // trying to do this beforehand can cause dealock on system locks such as the LoaderLock
        while (maxThreadsSimultaneosulyInProcessBuffer < 2)
        { // want to see more than one thread in this function, in order to verify that
            // the BufferFull registered by PIN_DefineTraceBuffer can be executed simultaneously
            // by different threads (BufferFull calls this function)
        }
    }

    if (!KnobProcessBuffer)
    {
        PIN_GetLock(&lock1, tid + 1);
        numThreadsSimultaneosulyInProcessBuffer--;
        PIN_ReleaseLock(&lock1);
        return;
    }

    UINT64 until = numElements;
    int numLoops = 20;
    for (int h = 0; h < numLoops; h++)
    { // make processing take a long time
        struct MEMREF* memref      = (struct MEMREF*)buf;
        struct MEMREF* firstMemref = memref;
        for (UINT64 i = 0; i < until; i++, memref++)
        {
            firstMemref->pc += memref->pc + memref->ea;
            if (memref->tid != tid)
            {
                printf("***Error unexpected tid %d in buffer of thread with tid %d\n", memref->tid, tid);
                exit(-1);
            }
        }
    }

    _numElementsProcessed += (UINT32)until;
    PIN_GetLock(&lock1, tid + 1);
    numThreadsSimultaneosulyInProcessBuffer--;
    PIN_ReleaseLock(&lock1);
    //printf ("numElements processed %d\n", (UINT32)numElements);
}

/*
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
                                     memOp, offsetof(struct MEMREF, ea), IARG_THREAD_ID, offsetof(struct MEMREF, tid), IARG_END);
            }
        }
    }
}

PIN_LOCK lock2;
void CalledWaitForAllThreadsStarted(THREADID tid)
{
    PIN_GetLock(&lock2, tid + 1);
    numThreadsCalledWaitForAllThreadsStarted++;
    PIN_ReleaseLock(&lock2);
}

static void InstrumentRtn(RTN rtn, VOID*)
{
    if (RTN_Name(rtn) == "WaitForAllThreadsStarted")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(CalledWaitForAllThreadsStarted), IARG_THREAD_ID, IARG_END);
        RTN_Close(rtn);
        printf("WaitForAllThreadsStarted instrumented\n");
        fflush(stdout);
    }
}

VOID Image(IMG img, void* v)
{
    printf("\nImage Image %s \n", IMG_Name(img).c_str());
    fflush(stdout);
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            printf("  rtn %s\n", RTN_Name(rtn).c_str());
        }
    }
}

/**************************************************************************
 *
 *  Callback Routines
 *
 **************************************************************************/

/*!
 * Called when a buffer fills up, or the thread exits, so we can process it or pass it off
 * as we see fit.
 * @param[in] id		buffer handle
 * @param[in] tid		id of owning thread
 * @param[in] ctxt		application context
 * @param[in] buf		actual pointer to buffer
 * @param[in] numElements	number of records
 * @param[in] v			callback value
 * @return  A pointer to the buffer to resume filling.
 */
VOID* BufferFull(BUFFER_ID id, THREADID tid, const CONTEXT* ctxt, VOID* buf, UINT64 numElements, VOID* v)
{
    APP_THREAD_REPRESENTITVE* appThreadRepresentitive =
        static_cast< APP_THREAD_REPRESENTITVE* >(PIN_GetThreadData(appThreadRepresentitiveKey, tid));

    appThreadRepresentitive->ProcessBuffer(buf, numElements, tid);

    return buf;
}

VOID ThreadStart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    // There is a new APP_THREAD_REPRESENTITVE for every thread.
    APP_THREAD_REPRESENTITVE* appThreadRepresentitive = new APP_THREAD_REPRESENTITVE(tid);

    // A thread will need to look up its APP_THREAD_REPRESENTITVE, so save pointer in TLS
    PIN_SetThreadData(appThreadRepresentitiveKey, appThreadRepresentitive, tid);
}

VOID ThreadFini(THREADID tid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    APP_THREAD_REPRESENTITVE* appThreadRepresentitive =
        static_cast< APP_THREAD_REPRESENTITVE* >(PIN_GetThreadData(appThreadRepresentitiveKey, tid));
    totalBuffersFilled += appThreadRepresentitive->NumBuffersFilled();
    totalElementsProcessed += appThreadRepresentitive->NumElementsProcessed();

    delete appThreadRepresentitive;

    PIN_SetThreadData(appThreadRepresentitiveKey, 0, tid);
}

VOID Fini(INT32 code, VOID* v)
{
    printf("maxThreadsSimultaneosulyInProcessBuffer %d totalBuffersFilled %u  totalElementsProcessed %14.0f\n",
           maxThreadsSimultaneosulyInProcessBuffer, (totalBuffersFilled), static_cast< double >(totalElementsProcessed));
    if (maxThreadsSimultaneosulyInProcessBuffer < 2)
    {
        printf("***Error - expected to have a number of threads simultaneously in the BufferFull function nesting\n");
        exit(-1);
    }
}

INT32 Usage()
{
    printf("This tool demonstrates simple pin-tool buffer managing\n");
    printf("The following command line options are available:\n");
    printf("-num_pages_in_buffer <num>   :number of (4096byte) pages allocated in each buffer,         default 256\n");
    printf("-process_buffs <0 or 1>      :specify 0 to disable processing of the buffers,              default   1\n");
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
    // prepare for image instrumentation mode
    PIN_InitSymbols();

    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    PIN_InitLock(&lock1);
    PIN_InitLock(&lock2);
    // Initialize the memory reference buffer
    //printf ("buffer size in bytes 0x%x\n", KnobNumPagesInBuffer.Value()*4096);
    //	fflush (stdout);

    bufId = PIN_DefineTraceBuffer(sizeof(struct MEMREF), KnobNumPagesInBuffer, BufferFull, 0);

    if (bufId == BUFFER_ID_INVALID)
    {
        printf("Error: could not allocate initial buffer\n");
        return 1;
    }

    // Initialize thread-specific data not handled by buffering api.
    appThreadRepresentitiveKey = PIN_CreateThreadDataKey(0);

    TRACE_AddInstrumentFunction(Trace, 0);

    RTN_AddInstrumentFunction(InstrumentRtn, 0);

    //IMG_AddInstrumentFunction(Image, 0);

    // add callbacks
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

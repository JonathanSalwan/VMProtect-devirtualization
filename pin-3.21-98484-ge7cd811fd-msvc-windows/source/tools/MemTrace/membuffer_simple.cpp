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
 * This tool is similar to memtrace_simple, but uses the Pin Buffering API
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
};

// The buffer ID returned by the one call to PIN_DefineTraceBuffer
BUFFER_ID bufId;

// the Pin TLS slot that an application-thread will use to hold the APP_THREAD_REPRESENTITVE
// object that it owns
TLS_KEY appThreadRepresentitiveKey;

UINT32 totalBuffersFilled     = 0;
UINT64 totalElementsProcessed = 0;

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

    VOID ProcessBuffer(VOID* buf, UINT64 numElements);
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

VOID APP_THREAD_REPRESENTITVE::ProcessBuffer(VOID* buf, UINT64 numElements)
{
    _numBuffersFilled++;
    //printf ("numElements %d\n", (UINT32)numElements);

    if (!KnobProcessBuffer)
    {
        return;
    }

    struct MEMREF* memref      = (struct MEMREF*)buf;
    struct MEMREF* firstMemref = memref;
    UINT64 until               = numElements;
    for (UINT64 i = 0; i < until; i++, memref++)
    {
        firstMemref->pc += memref->pc + memref->ea;
    }
    _numElementsProcessed += (UINT32)until;
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

    appThreadRepresentitive->ProcessBuffer(buf, numElements);

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
    return;
    printf("totalBuffersFilled %u  totalElementsProcessed %14.0f\n", (totalBuffersFilled),
           static_cast< double >(totalElementsProcessed));
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
    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

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

    // add an instrumentation function
    TRACE_AddInstrumentFunction(Trace, 0);

    // add callbacks
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *
 * This tool collects an address trace of instructions that access memory
 * by filling a per-thread buffer. Each memory access of an instruction is recorded
 * as a MEMREF in the buffer (see MEMREF below).
 *

 * We do TRACE based instrumentation. At the top of the TRACE, we allocate
 * the maximum amount of space that might be needed for this trace in the
 * buffer. At each memory instruction, we just record the MEMREF at a
 * pre-determined offset in the buffer. If there are early exits from a trace,
 * then MEMREF records will not be placed in the buffer for INSs that follow the
 * early exit - We recognize these by initializing the buffer to 0 - so that the
 * entries in the buffer which were not filled in will have invalid values for both
 * pc and ea of any MEMREF
 *
 * We check if the buffer is full at the top of the trace. If it is full, we
 * process the buffer and reset the buffer pointer.
 *
 * Each application thread has it's own buffer - so multiple application threads do NOT
 * block each other on buffer accesses
 *
 *  This tool is similar to membuffer_simple, but does NOT use the Pin Buffering API, it
 *  manages the buffers on it's own
 *
 */

#include <cassert>
#include <cstdio>
#include <map>
#include <set>
#include "pin.H"
using std::vector;

/*
 *
 *
 *
 * Knobs for tool
 *
 *
 *
 */

/*
 * Emit the address trace to the output file
 */
KNOB< BOOL > KnobProcessBuffer(KNOB_MODE_WRITEONCE, "pintool", "process_buffs", "1", "process the filled buffers");

// 256*4096=1048576 - same size buffer in memtrace_simple, membuffer_simple, membuffer_multi
KNOB< UINT32 > KnobNumBytesInBuffer(KNOB_MODE_WRITEONCE, "pintool", "num_bytes_in_buffer", "1048576",
                                    "number of bytes in buffer");

/* Struct of memory reference written to the buffer
 */
struct MEMREF
{
    ADDRINT pc; // pc (ip) of the instruction doing the memory reference
    ADDRINT ea; // the address of the memory being referenced
};

// the Pin TLS slot that an application-thread will use to hold the APP_THREAD_BUFFER_HANDLER
// object that it owns
TLS_KEY appThreadRepresentitiveKey;

UINT32 totalBuffersFilled     = 0;
UINT64 totalElementsProcessed = 0;

/* Pin registers that this tool allocates (per-thread) to manage the writing
 * to the per-thread buffer
 */
REG endOfTraceInBufferReg;
REG endOfBufferReg;
/*
 *
 * APP_THREAD_REPRESENTITVE
 *
 * Each application thread, creates an object of this class and saves it in it's Pin TLS
 * slot (appThreadRepresentitiveKey).
 * The class provides the management of the per thread buffer, including the
 * analysis routines to be used
 */
class APP_THREAD_REPRESENTITVE
{
  public:
    APP_THREAD_REPRESENTITVE(THREADID tid);
    ~APP_THREAD_REPRESENTITVE();

    /*
     * ProcessBuffer
     */
    void ProcessBuffer(char* end);

    /*
     * Pointer to beginning of the buffer
     */
    char* Begin() { return _buffer; }

    /*
     * Pointer to end of the buffer
     */
    char* End() { return _buffer + KnobNumBytesInBuffer.Value(); }

    UINT32 NumBuffersFilled() { return _numBuffersFilled; }

    UINT32 NumElementsProcessed() { return _numElementsProcessed; }

    /*
     * Analysis routine to record a MEMREF (pc, ea) in the buffer
     */
    static void PIN_FAST_ANALYSIS_CALL RecordMEMREFInBuffer(CHAR* endOfTraceInBuffer, ADDRINT offsetFromEndOfTrace, ADDRINT pc,
                                                            ADDRINT ea)
    {
        *reinterpret_cast< ADDRINT* >(endOfTraceInBuffer + offsetFromEndOfTrace)                   = pc;
        *reinterpret_cast< ADDRINT* >(endOfTraceInBuffer + offsetFromEndOfTrace + sizeof(ADDRINT)) = ea;
    }

    /*
	 * Analysis routine called at beginning of each trace - it is the IF part of the IF THEN analysis routine pair
     * Return 0 if there is room in the buffer for the current trace
     *
     * @param[in]   endOfPreviousTraceInBuffer     Pointer to next entry in the buffer
     * @param[in]   bufferEnd                      Pointer to end of the buffer
     * @param[in]   totalSizeOccupiedByTraceInBuffer Number of bytes required by this TRACE
     */
    static ADDRINT PIN_FAST_ANALYSIS_CALL CheckIfNoSpaceForTraceInBuffer(char* endOfPreviousTraceInBuffer, char* bufferEnd,
                                                                         ADDRINT totalSizeOccupiedByTraceInBuffer)
    {
        return (endOfPreviousTraceInBuffer + totalSizeOccupiedByTraceInBuffer >= bufferEnd);
    }

    /*
	 * Analysis routine called when the above IF part returns 1 - this the THEN part
     * The buffer does not have room for this trace, process the current buffer entries
     * and restart to fill the buffer from it's beginning
     */
    static char* PIN_FAST_ANALYSIS_CALL BufferFull(char* endOfTraceInBuffer, ADDRINT tid)
    {
        APP_THREAD_REPRESENTITVE* appThreadRepresentitive =
            static_cast< APP_THREAD_REPRESENTITVE* >(PIN_GetThreadData(appThreadRepresentitiveKey, tid));
        appThreadRepresentitive->ProcessBuffer(endOfTraceInBuffer);
        endOfTraceInBuffer = appThreadRepresentitive->Begin();

        return endOfTraceInBuffer;
    }

    /*
     * Analysis routine called at beginning of each trace (after the IF-THEN)-
	 * moves the endOfPreviousTraceInBuffer further down in the buffer to allocate space for all
	 * the possible MEMREF elements that may be written by this trace
     */
    static char* PIN_FAST_ANALYSIS_CALL AllocateSpaceForTraceInBuffer(char* endOfPreviousTraceInBuffer,
                                                                      ADDRINT totalSizeOccupiedByTraceInBuffer)
    {
        return (endOfPreviousTraceInBuffer + totalSizeOccupiedByTraceInBuffer);
    }

  private:
    /*
     * Reset curMEMREFElement to the beginning of the buffer
     */
    void ResetCurMEMREFElement(char** curMEMREFElement);

    char* _buffer; // this is the actual buffer
    UINT32 _numBuffersFilled;
    UINT32 _numElementsProcessed;
    THREADID _myTid;
};

/*
 * ANALYSIS_CALL_INFO
 *
 * Analysis calls that must be inserted at an INS in the trace are recorded in an
 * ANALYSIS_CALL_INFO object
 *
 */
class ANALYSIS_CALL_INFO
{
  public:
    ANALYSIS_CALL_INFO(INS ins, UINT32 offsetFromTraceStartInBuffer, UINT32 memop)
        : _ins(ins), _offsetFromTraceStartInBuffer(offsetFromTraceStartInBuffer), _memop(memop)
    {}

    void InsertAnalysisCall(INT32 sizeofTraceInBuffer)
    {
        /* the place in the buffer where the MEMREF of this _ins should be recorded is
           computed by: (the value in endOfTraceInBufferReg)
                        -sizeofTraceInBuffer +  _offsetFromTraceStartInBuffer(of this _ins)
        */
        INS_InsertCall(_ins, IPOINT_BEFORE, AFUNPTR(APP_THREAD_REPRESENTITVE::RecordMEMREFInBuffer), IARG_FAST_ANALYSIS_CALL,
                       IARG_REG_VALUE, endOfTraceInBufferReg, IARG_ADDRINT,
                       ADDRINT(_offsetFromTraceStartInBuffer - sizeofTraceInBuffer), IARG_INST_PTR, IARG_MEMORYOP_EA, _memop,
                       IARG_END);
    }

  private:
    INS _ins;
    INT32 _offsetFromTraceStartInBuffer;
    UINT32 _memop;
};

/*
 * TRACE_ANALYSIS_CALLS_NEEDED
 *
 * Information about what analysis calls must be inserted at INSs in the trace are recorded here
 * This info is held in a vector of ANALYSIS_CALL_INFO objects
 *
 */
class TRACE_ANALYSIS_CALLS_NEEDED
{
  public:
    TRACE_ANALYSIS_CALLS_NEEDED() : _currentOffsetFromTraceStartInBuffer(0), _numAnalysisCallsNeeded(0) {}

    UINT32 NumAnalysisCallsNeeded() const { return _numAnalysisCallsNeeded; }

    UINT32 TotalSizeOccupiedByTraceInBuffer() const { return _currentOffsetFromTraceStartInBuffer; }

    /*
     * Record a call to store an address in the log
     */
    void RecordAnalysisCallNeeded(INS ins, UINT32 memop);

    /*
     * InsertAnalysisCall all the recorded necessary analysis calls into the trace
     */
    void InsertAnalysisCalls();

  private:
    INT32 _currentOffsetFromTraceStartInBuffer;
    INT32 _numAnalysisCallsNeeded;
    vector< ANALYSIS_CALL_INFO > _analysisCalls;
};

APP_THREAD_REPRESENTITVE::APP_THREAD_REPRESENTITVE(THREADID myTid)
{
    _buffer               = new char[KnobNumBytesInBuffer.Value()];
    _numBuffersFilled     = 0;
    _numElementsProcessed = 0;
    _myTid                = myTid;
}

APP_THREAD_REPRESENTITVE::~APP_THREAD_REPRESENTITVE() { delete[] _buffer; }

/*
 * Process the buffer
 */
void APP_THREAD_REPRESENTITVE::ProcessBuffer(char* end)
{
    _numBuffersFilled++;
    if (!KnobProcessBuffer)
    {
        return;
    }
    int i                      = 0;
    struct MEMREF* memref      = reinterpret_cast< struct MEMREF* >(Begin());
    struct MEMREF* firstMemref = memref;

    while (memref < reinterpret_cast< struct MEMREF* >(end))
    {
        if (memref->pc != 0)
        {
            i++;
            firstMemref->pc += memref->pc + memref->ea;
            memref->pc = 0;
        }
        memref++;
    }

    //printf ("numElements %d (full %d   empty %d)\n", i+j, i, j);
    _numElementsProcessed += i;
}

/*
 * Reset the cursor to the beginning of the APP_THREAD_REPRESENTITVE
 */
void APP_THREAD_REPRESENTITVE::ResetCurMEMREFElement(char** curMEMREFElement) { *curMEMREFElement = Begin(); }

/*
 * We determined all the required instrumentation, insert the calls
 */
void TRACE_ANALYSIS_CALLS_NEEDED::InsertAnalysisCalls()
{
    for (vector< ANALYSIS_CALL_INFO >::iterator c = _analysisCalls.begin(); c != _analysisCalls.end(); c++)
    {
        c->InsertAnalysisCall(TotalSizeOccupiedByTraceInBuffer());
    }
}

/*
 * Record that we need to insert an analysis call to gather the MEMREF info for this ins
 */
void TRACE_ANALYSIS_CALLS_NEEDED::RecordAnalysisCallNeeded(INS ins, UINT32 memop)
{
    _analysisCalls.push_back(ANALYSIS_CALL_INFO(ins, _currentOffsetFromTraceStartInBuffer, memop));
    _currentOffsetFromTraceStartInBuffer += sizeof(MEMREF);
    _numAnalysisCallsNeeded++;
}

void DetermineBBLAnalysisCalls(BBL bbl, TRACE_ANALYSIS_CALLS_NEEDED* traceAnalysisCallsNeeded)
{
    // Log  memory references of the instruction
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
    {
        UINT32 memOperands = INS_MemoryOperandCount(ins);

        // Iterate over each memory operand of the instruction.
        for (UINT32 memOp = 0; memOp < memOperands; memOp++)
        {
            traceAnalysisCallsNeeded->RecordAnalysisCallNeeded(ins, memOp);
        }
    }
}

void TraceAnalysisCalls(TRACE trace, void*)
{
    // Go over all BBLs of the trace and for each BBL determine and record the INSs which need
    // to be instrumented - i.e. the ins requires and analysis call
    //
    TRACE_ANALYSIS_CALLS_NEEDED traceAnalysisCallsNeeded;
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        DetermineBBLAnalysisCalls(bbl, &traceAnalysisCallsNeeded);
    }

    // No addresses in this trace
    if (traceAnalysisCallsNeeded.NumAnalysisCallsNeeded() == 0)
    {
        return;
    }

    // Now we know how bytes the analysis calls of this trace will insert into the buffer
    //   Each analysis call inserts a MEMREF into the buffer

    // APP_THREAD_REPRESENTITVE::CheckIfNoSpaceForTraceInBuffer will determine if there are NOT enough available bytes in the buffer.
    // If there are NOT then it returns TRUE and the BufferFull function is called
    TRACE_InsertIfCall(trace, IPOINT_BEFORE, AFUNPTR(APP_THREAD_REPRESENTITVE::CheckIfNoSpaceForTraceInBuffer),
                       IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, endOfTraceInBufferReg, // previous trace
                       IARG_REG_VALUE, endOfBufferReg, IARG_UINT32, traceAnalysisCallsNeeded.TotalSizeOccupiedByTraceInBuffer(),
                       IARG_END);
    TRACE_InsertThenCall(trace, IPOINT_BEFORE, AFUNPTR(APP_THREAD_REPRESENTITVE::BufferFull), IARG_FAST_ANALYSIS_CALL,
                         IARG_REG_VALUE, endOfTraceInBufferReg, IARG_THREAD_ID, IARG_RETURN_REGS, endOfTraceInBufferReg,
                         IARG_END);
    TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(APP_THREAD_REPRESENTITVE::AllocateSpaceForTraceInBuffer),
                     IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, endOfTraceInBufferReg, IARG_UINT32,
                     traceAnalysisCallsNeeded.TotalSizeOccupiedByTraceInBuffer(), IARG_RETURN_REGS, endOfTraceInBufferReg,
                     IARG_END);

    // Insert Analysis Calls for each INS on the trace that was recorded as needing one
    //   i.e. each INS that reads and/or writes memory
    traceAnalysisCallsNeeded.InsertAnalysisCalls();
}

VOID ThreadStart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    // There is a new APP_THREAD_REPRESENTITVE for every thread
    APP_THREAD_REPRESENTITVE* appThreadRepresentitive = new APP_THREAD_REPRESENTITVE(tid);

    // A thread will need to look up its APP_THREAD_REPRESENTITVE, so save pointer in TLS
    PIN_SetThreadData(appThreadRepresentitiveKey, appThreadRepresentitive, tid);

    // Initialize endOfTraceInBufferReg to point at beginning of buffer
    PIN_SetContextReg(ctxt, endOfTraceInBufferReg, reinterpret_cast< ADDRINT >(appThreadRepresentitive->Begin()));

    // Initialize endOfBufferReg to point at end of buffer
    PIN_SetContextReg(ctxt, endOfBufferReg, reinterpret_cast< ADDRINT >(appThreadRepresentitive->End()));
}

VOID ThreadFini(THREADID tid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    APP_THREAD_REPRESENTITVE* appThreadRepresentitive =
        static_cast< APP_THREAD_REPRESENTITVE* >(PIN_GetThreadData(appThreadRepresentitiveKey, tid));
    appThreadRepresentitive->ProcessBuffer(reinterpret_cast< char* >(PIN_GetContextReg(ctxt, endOfTraceInBufferReg)));
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
    printf("-num_bytes_in_buffer <num>   :number of bytes allocated for each buffer,                   default 1048576\n");
    printf("-process_buffs <0 or 1>      :specify 0 to disable processing of the buffers,              default       1\n");
    return -1;
}

int main(int argc, char* argv[])
{
    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    appThreadRepresentitiveKey = PIN_CreateThreadDataKey(0);

    // get the registers to be used in each thread for managing the
    // per-thread buffer
    endOfTraceInBufferReg = PIN_ClaimToolRegister();
    endOfBufferReg        = PIN_ClaimToolRegister();

    if (!(REG_valid(endOfTraceInBufferReg) && REG_valid(endOfBufferReg)))
    {
        printf("Cannot allocate a scratch register.\n");
        return 1;
    }

    TRACE_AddInstrumentFunction(TraceAnalysisCalls, 0);
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);

    PIN_AddFiniFunction(Fini, 0);

    //printf ("buffer size in bytes 0x%x\n", KnobNumBytesInBuffer.Value());
    // fflush (stdout);
    PIN_StartProgram();

    return 0;
}

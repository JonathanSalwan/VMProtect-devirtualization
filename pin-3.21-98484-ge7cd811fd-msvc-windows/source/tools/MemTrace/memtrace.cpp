/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Collect an address trace
 *
 * During execution of the program, record values in a MLOG and use the
 * MLOG values to reconstuct the trace. The MLOG can contain the actual
 * addresses or register values that can be used to compute the
 * address. This tool is thread safe. Each thread writes to its own MLOG
 * and each MLOG is dumped to a separate file.
 *
 */

/*
 * We do TRACE based instrumentation. At the top of the TRACE, we allocate
 * the maximum amount of space that might be needed for this trace in the
 * log. At each memory instruction, we just record the address at a
 * pre-determined slot in the log. If there are early exits from a trace,
 * then their slots will be empty. We initialize all the empty slots to an
 * invalid address so we can tell later that this instruction did not
 * reference memory.
 *
 * We check if the log is full at the top of the trace. If it is full, we
 * empty the log and reset the log pointer.
 *
 */
#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <unistd.h>
#include "pin.H"
using std::endl;
using std::hex;
using std::ofstream;
using std::string;
using std::vector;

#define MEMTRACE_DEBUG 0

TLS_KEY mlog_key;
REG scratch_reg0;
REG scratch_reg1;

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
 * Name of the output file
 */
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "memtrace.out", "output file");

/*
 * Emit the address trace to the output file
 */
KNOB< BOOL > KnobEmitTrace(KNOB_MODE_WRITEONCE, "pintool", "emit", "0", "emit a trace in the output file");

/*
 *
 *
 *
 * Forward type definitions
 *
 *
 *
 */
class TRACE_HEADER;

/*
 *
 *
 *
 * Static info about the memory reference
 *
 *
 *
 */
class REF
{
  public:
    REF() {};

    REF(ADDRINT ip, BOOL read)
    {
        _ip   = ip;
        _read = read;
    }

    ADDRINT IP() const { return _ip; }

  private:
    // ip of instruction making reference
    ADDRINT _ip;

    // is it a memory read
    BOOL _read;
};

/*
 * Instrumentation call
 *
 * We need to insert the instrumentation calls in the order that they
 * should be called, so we record all the calls first and insert them in
 * the proper order later.
 *
 */
class CALL
{
  public:
    CALL(INS ins, AFUNPTR afunptr, UINT32 offset, IARG_TYPE itype)
    {
        _ins     = ins;
        _afunptr = afunptr;
        _offset  = offset;
        _itype   = itype;
    }

    void Insert(INT32 frameSize)
    {
        INS_InsertCall(_ins, IPOINT_BEFORE, _afunptr, IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, scratch_reg0, IARG_ADDRINT,
                       ADDRINT(_offset - frameSize), _itype, IARG_END);
    }

  private:
    INS _ins;
    AFUNPTR _afunptr;
    INT32 _offset;
    IARG_TYPE _itype;
};

/*
 * These are the types of commands used to interpret the entries in the
 * MLOG to reconstruct the trace
 */
typedef enum
{
    TCMD_INVALID,

    // trace entry is effective address
    TCMD_IMMEDIATE,

    // trace entry is register value
    TCMD_REG_VALUE,

    // no trace entry, address is offset from register value
    TCMD_REG_OFFSET,

    // no trace entry, address of offset from previous trace entry
    TCMD_TRACE_OFFSET,

    // if trace entry is 0, basic block is not executed, abort
    TCMD_BBLOCK

} TCMD_TYPE;

/*
 *
 * Temporarily store trace header information until all information is
 * known, then a more compact one is generated from this
 *
 */
class SCRATCH_TRACE_HEADER
{
    friend class TRACE_HEADER;

  public:
    SCRATCH_TRACE_HEADER()
    {
        // The first element of the log is the trace header so leave space for it
        _frameOffset = sizeof(TRACE_HEADER*);
    };

    UINT32 NumCmds() const { return _cmds.size(); }

    /*
     * Record a call to store an address in the log
     */
    void RecordLogImmediate(INS ins, IARG_TYPE itype);

    /*
     * Insert all the recorded calls into the trace
     */
    void InsertCalls();

  private:
    INT32 _frameOffset;
    vector< TCMD_TYPE > _cmds;
    vector< REF > _refs;
    vector< CALL > _calls;
};

/*
 *
 * Permanent trace header storage
 *
 */
class TRACE_HEADER
{
  public:
    TRACE_HEADER(SCRATCH_TRACE_HEADER* header);
    void Expand(char* traceBuffer, char* log);
    int LogBytes() const { return _logBytes; }
    int NumCmds() const { return _numCmds; }
    TCMD_TYPE CmdType(int i) const
    {
        assert(i < NumCmds());
        return _cmds[i];
    }
    REF const* Ref(int i) const
    {
        assert(i < NumCmds());
        return _refs + i;
    }

  private:
    int _logBytes;
    int _numCmds;
    TCMD_TYPE* _cmds;
    REF* _refs;
};

/*
 *
 * MLOG
 *
 * Instrumentation records information into a MLOG. This information is used
 * with the TRACE_HEADER to construct the address trace.
 *
 *
 */
class MLOG
{
  private:
    enum
    {
        BUFFER_SIZE = 1000000,
        EMPTY_SLOT  = 0
    };

  public:
    MLOG(THREADID tid);
    ~MLOG();

    /*
     * Record an address immediate in log
     */
    static void PIN_FAST_ANALYSIS_CALL RecordImmediate(CHAR* logCursor, ADDRINT offset, ADDRINT value)
    {
#if MEMTRACE_DEBUG > 10
        fprintf(stderr, "LogImmediate %p logCursor %p offset %p\n", logCursor + offset, logCursor, offset);
#endif

        *reinterpret_cast< ADDRINT* >(logCursor + offset) = value;
    }

    static ADDRINT PIN_FAST_ANALYSIS_CALL TraceAllocIf(char* logCursor, char* logEnd, ADDRINT size);
    static char* PIN_FAST_ANALYSIS_CALL TraceAllocThen(char* logCursor, ADDRINT size, THREADID tid);
    static char* PIN_FAST_ANALYSIS_CALL RecordTraceBegin(char* logCursor, TRACE_HEADER* theader, ADDRINT size);

    /*
     * Add a trace header to the log and return next logCursor
     */
    static char* PIN_FAST_ANALYSIS_CALL BeginTrace(char* logCursor, char* logEnd, TRACE_HEADER* theader, UINT32 size,
                                                   THREADID tid);

    /*
     * Expand the log into an address trace
     */
    void Expand();

    /*
     * Pointer to beginning of log data
     */
    char* Begin() { return _data; }

    /*
     * Pointer to end of log data
     */
    char* End() { return _data + BUFFER_SIZE; }

  private:
    ofstream ofile;

    /*
     * Reset log cursor to the beginning of the buffer
     */
    void ResetLogCursor(char** logCursor);

    /*
     * Return true if position in log is empty
     */
    static BOOL EmptySlot(char* logCursor) { return *reinterpret_cast< ADDRINT* >(logCursor) == EMPTY_SLOT; }

    /*
     * Mark this position in log as empty
     */
    static void MarkEmptySlot(char* logCursor) { *reinterpret_cast< ADDRINT* >(logCursor) = EMPTY_SLOT; }

    static ADDRINT Immediate(char* logCursor) { return *reinterpret_cast< ADDRINT* >(logCursor); }

    static TRACE_HEADER const* TraceHeader(char* logCursor) { return *reinterpret_cast< TRACE_HEADER const** >(logCursor); }

    static void RecordTraceHeader(char* logCursor, TRACE_HEADER* traceHeader)
    {
        *reinterpret_cast< TRACE_HEADER const** >(logCursor) = traceHeader;
    }

    /*
     * Mark all the slots in the log as empty
     */
    void Reset();

    int NumLogBytes(TCMD_TYPE ttype);

    void ExpandTrace(TRACE_HEADER const* traceHeader, char* logCursor);

    char* _data;
};

MLOG::MLOG(THREADID tid)
{
    if (KnobEmitTrace)
    { // no need for output file if nothing is being emitted
        const string filename = KnobOutputFile.Value() + "." + decstr(getpid()) + "." + decstr(tid);
        // Open the memtrace file
        ofile.open(filename.c_str());
        ofile << hex;
    }

    _data = new char[BUFFER_SIZE];
    Reset();
}

MLOG::~MLOG()
{
    // Flush remaining data from buffer
    Expand();

    if (KnobEmitTrace)
    {
        ofile.close();
    }

    delete[] _data;
}

/*
 * Mark all the slots in the log as empty
 */
void MLOG::Reset()
{
    for (int i = 0; i < BUFFER_SIZE; i += sizeof(ADDRINT))
    {
        MarkEmptySlot(_data + i);
    }
}

/*
 * Interpret all the commands and MLOG entries to generate the addresses
 * for a single TRACE
 */
void MLOG::ExpandTrace(TRACE_HEADER const* traceHeader, char* logCursor)
{
    int ref = 0;
    for (int cmd = 0; cmd < traceHeader->NumCmds(); cmd++)
    {
        switch (traceHeader->CmdType(cmd))
        {
            case TCMD_IMMEDIATE:
                if (KnobEmitTrace && (!EmptySlot(logCursor)))
                {
                    ofile << traceHeader->Ref(ref)->IP() << " " << Immediate(logCursor) << endl;
                }
                ref++;
                break;

            default:
                assert(false);
        }

        // Advance the log cursor
        logCursor += NumLogBytes(traceHeader->CmdType(cmd));
    }
}

/*
 * The log has filled, generate the address trace
 */
void MLOG::Expand()
{
#if MEMTRACE_DEBUG > 10
    fprintf(stderr, "WriteLog\n");
#endif

    if (!KnobEmitTrace) return;

    TRACE_HEADER const* theader = 0;

    for (char* logCursor = Begin(); !EmptySlot(logCursor); logCursor += theader->LogBytes())
    {
        // Read the trace header
        theader = TraceHeader(logCursor);

        // Use the log to construct the memory trace
        ExpandTrace(theader, logCursor + sizeof(TRACE_HEADER*));
    }

    Reset();
}

/*
 * Return 0 if the MLOG has room for this TRACE
 *
 * @param[in]   logCursor   Pointer to next entry in MLOG to be used
 * @param[in]   logEnd      Pointer to end of MLOG buffer
 * @param[in]   size        Number of bytes required by this TRACE
 */
ADDRINT MLOG::TraceAllocIf(char* logCursor, char* logEnd, ADDRINT size) { return (logCursor + size >= logEnd); }

/*
 * The MLOG does not have room for the next TRACE, generate all the addresses and reset
 */
char* MLOG::TraceAllocThen(char* logCursor, ADDRINT size, THREADID tid)
{
    MLOG* mlog = static_cast< MLOG* >(PIN_GetThreadData(mlog_key, tid));

    // If adding this trace will exceed the buffer size then flush the log
    assert(logCursor + size >= mlog->End());
    mlog->Expand();
    mlog->ResetLogCursor(&logCursor);

    return logCursor;
}

/*
 * Record the TRACE_HEADER and advance MLOG cursor
 */
char* MLOG::RecordTraceBegin(char* logCursor, TRACE_HEADER* theader, ADDRINT size)
{
    // Record the trace header in the log
    RecordTraceHeader(logCursor, theader);

    // Advance the log pointer
    return logCursor + size;
}

/*
 * Processing for the beginning of a Pin TRACE
 *
 * Check if the MLOG has room for this trace. If not, empty the
 * MLOG. Record the trace header and advance the pointer to where the next
 * trace header should go.
 *
 */
char* MLOG::BeginTrace(char* logCursor, char* logEnd, TRACE_HEADER* theader, UINT32 size, THREADID tid)
{
#if MEMTRACE_DEBUG > 10
    fprintf(stderr, "LogTraceBegin %p\n", logCursor);
#endif

    if (TraceAllocIf(logCursor, logEnd, size))
    {
        logCursor = TraceAllocThen(logCursor, size, tid);
    }

    logCursor = RecordTraceBegin(logCursor, theader, size);

    return logCursor;
}

/*
 * Number of bytes in the MLOG that this command consumes
 */
int MLOG::NumLogBytes(TCMD_TYPE ttype)
{
    switch (ttype)
    {
        case TCMD_IMMEDIATE:
            return sizeof(ADDRINT);

        default:
            assert(false);
            return 0;
    }
}

/*
 * Reset the cursor to the beginning of the MLOG
 */
void MLOG::ResetLogCursor(char** logCursor) { *logCursor = Begin(); }

TRACE_HEADER::TRACE_HEADER(SCRATCH_TRACE_HEADER* scratch)
{
    _logBytes = scratch->_frameOffset;

    _numCmds = scratch->_cmds.size();

    _cmds = new TCMD_TYPE[scratch->_cmds.size()];
    for (UINT32 i = 0; i < scratch->_cmds.size(); i++)
    {
        _cmds[i] = scratch->_cmds[i];
    }

    _refs = new REF[scratch->_refs.size()];
    for (UINT32 i = 0; i < scratch->_refs.size(); i++)
    {
        _refs[i] = scratch->_refs[i];
    }
}

/*
 * We determined all the required instrumentation, insert the calls
 */
void SCRATCH_TRACE_HEADER::InsertCalls()
{
    for (vector< CALL >::iterator c = _calls.begin(); c != _calls.end(); c++)
    {
        c->Insert(_frameOffset);
    }
}

/*
 * Record that we need to insert instrumentation to record an immediate
 */
void SCRATCH_TRACE_HEADER::RecordLogImmediate(INS ins, IARG_TYPE itype)
{
    _cmds.push_back(TCMD_IMMEDIATE);
    _refs.push_back(REF(INS_Address(ins), itype != IARG_MEMORYWRITE_EA));
    _calls.push_back(CALL(ins, AFUNPTR(MLOG::RecordImmediate), _frameOffset, itype));
    _frameOffset += sizeof(ADDRINT);

#if MEMTRACE_DEBUG > 5
    fprintf(stderr, "InsertLogImmediate ip %p\n", INS_Address(ins));
#endif
}

void InstrumentBBL(BBL bbl, SCRATCH_TRACE_HEADER* theader)
{
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
    {
        // Log every memory references of the instruction
        if (INS_IsMemoryRead(ins) && INS_IsStandardMemop(ins))
        {
            theader->RecordLogImmediate(ins, IARG_MEMORYREAD_EA);
        }
        if (INS_IsMemoryWrite(ins) && INS_IsStandardMemop(ins))
        {
            theader->RecordLogImmediate(ins, IARG_MEMORYWRITE_EA);
        }
        if (INS_HasMemoryRead2(ins) && INS_IsStandardMemop(ins))
        {
            theader->RecordLogImmediate(ins, IARG_MEMORYREAD2_EA);
        }
    }
}

void InstrumentTrace(TRACE trace, void*)
{
    // We have to determine how many bytes are needed in the MLOG to record
    // information for this trace, so make a pass to determine what type of
    // instrumentation is needed, but do not insert anything yet.
    //
    SCRATCH_TRACE_HEADER scratchHeader;
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        InstrumentBBL(bbl, &scratchHeader);
    }

    // No addresses in this trace
    if (scratchHeader.NumCmds() == 0) return;

    // Pack everything into a TRACE_HEADER
    TRACE_HEADER* theader = new TRACE_HEADER(&scratchHeader);

    // Now that we know how many MLOG bytes are needed, we can insert instrumentation
    // Insert call to allocate trace entries and write trace header to log
#define INSERT_IF
#if defined(INSERT_IF)
    // Instead of using G1 to hold the end, we could use known alignment to detect buffer end
    TRACE_InsertIfCall(trace, IPOINT_BEFORE, AFUNPTR(MLOG::TraceAllocIf), IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, scratch_reg0,
                       IARG_REG_VALUE, scratch_reg1, IARG_UINT32, theader->LogBytes(), IARG_END);
    TRACE_InsertThenCall(trace, IPOINT_BEFORE, AFUNPTR(MLOG::TraceAllocThen), IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE,
                         scratch_reg0, IARG_UINT32, theader->LogBytes(), IARG_RETURN_REGS, scratch_reg0, IARG_THREAD_ID,
                         IARG_END);
    TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(MLOG::RecordTraceBegin), IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, scratch_reg0,
                     IARG_PTR, theader, IARG_UINT32, theader->LogBytes(), IARG_RETURN_REGS, scratch_reg0, IARG_END);
#else
    TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(MLOG::BeginTrace), IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, scratch_reg0,
                     IARG_REG_VALUE, scratch_reg1, IARG_PTR, theader, IARG_UINT32, theader->LogBytes(), IARG_RETURN_REGS,
                     scratch_reg0, IARG_THREAD_ID, IARG_END);
#endif

    // Insert calls for the individual loads and stores
    scratchHeader.InsertCalls();
}

VOID ThreadStart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    // There is a new MLOG for every thread
    MLOG* mlog = new MLOG(tid);

    // A thread will need to look up its MLOG, so save pointer in TLS
    PIN_SetThreadData(mlog_key, mlog, tid);

    // Initialize cursor to point at beginning of buffer
    PIN_SetContextReg(ctxt, scratch_reg0, reinterpret_cast< ADDRINT >(mlog->Begin()));

    PIN_SetContextReg(ctxt, scratch_reg1, reinterpret_cast< ADDRINT >(mlog->End()));
}

VOID ThreadFini(THREADID tid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    MLOG* mlog = static_cast< MLOG* >(PIN_GetThreadData(mlog_key, tid));

    delete mlog;

    PIN_SetThreadData(mlog_key, 0, tid);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    mlog_key     = PIN_CreateThreadDataKey(0);
    scratch_reg0 = PIN_ClaimToolRegister();
    scratch_reg1 = PIN_ClaimToolRegister();

    if (!(REG_valid(scratch_reg0) && REG_valid(scratch_reg1)))
    {
        std::cerr << "Cannot allocate a scratch register.\n";
        std::cerr << std::flush;
        return 1;
    }

    TRACE_AddInstrumentFunction(InstrumentTrace, 0);
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);

    PIN_StartProgram();

    return 0;
}

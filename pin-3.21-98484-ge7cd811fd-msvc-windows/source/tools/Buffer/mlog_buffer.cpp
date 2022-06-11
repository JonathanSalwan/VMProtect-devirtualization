/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Sample buffering tool
 *
 * This tool collects an address trace of instructions that access memory
 * by filling a buffer.  When the buffer overflows,the callback writes all
 * of the collected records to a file.
 *
 * This tool does a similar task as memtrace.cpp, but it uses the buffering api.
 */
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstddef>
#include <unistd.h>
#include "pin.H"
using std::cerr;
using std::endl;
using std::flush;
using std::hex;
using std::ofstream;
using std::string;

/*
 * Knobs for tool
 */

/*
 * Name of the output file
 */
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "mlog_buffer.out", "output file");

/*
 * The ID of the buffer
 */
BUFFER_ID bufId;

/*
 * Thread specific data
 */
TLS_KEY mlog_key;

/*
 * Number of OS pages for the buffer
 */
#define NUM_BUF_PAGES 1024

/* Struct for holding memory references.  Rather than having two separate
 * buffers for reads and writes, we just use one struct that includes a
 * flag for type.
 */
struct MEMREF
{
    ADDRINT pc;
    ADDRINT ea;
    UINT32 size;
    BOOL read;
    UINT64 tsc;
};

/*
 * MLOG - thread specific data that is not handled by the buffering API.
 */
class MLOG
{
  public:
    MLOG() {}
    ~MLOG();

    VOID DumpBufferToFile(struct MEMREF* reference, UINT64 numElements, THREADID tid);

  private:
    ofstream _ofile;
};

MLOG::~MLOG()
{
    if (_ofile.is_open())
    {
        _ofile.close();
    }
}

VOID MLOG::DumpBufferToFile(struct MEMREF* reference, UINT64 numElements, THREADID tid)
{
    if (!_ofile.is_open())
    {
        const string filename = KnobOutputFile.Value() + "." + decstr(getpid()) + "." + decstr(tid);

        _ofile.open(filename.c_str());

        if (!_ofile.is_open())
        {
            cerr << "Error: could not open output file " << filename << endl;
            PIN_ExitProcess(1);
        }

        _ofile << hex;
    }

    for (UINT64 i = 0; i < numElements; i++, reference++)
    {
        if (reference->ea != 0)
        {
            _ofile << reference->pc << (reference->read ? "  R " : "  W ") << reference->ea << " : " << reference->tsc << endl;
        }
    }
}

/*!
 *  Print out help message.
 */
INT32 Usage()
{
    cerr << "This tool demonstrates the basic use of the buffering API." << endl << endl;

    return -1;
}

/*
 * Insert code to write data to a thread-specific buffer for instructions
 * that access memory.
 */
VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_MemoryOperandCount(ins) == 0) continue;

            UINT32 readSize = 0, read2Size = 0, writeSize = 0;
            UINT32 readOperandCount = 0, writeOperandCount = 0;

            for (UINT32 opIdx = 0; opIdx < INS_MemoryOperandCount(ins); opIdx++)
            {
                if (INS_MemoryOperandIsRead(ins, opIdx))
                {
                    if (readSize == 0)
                        readSize = INS_MemoryOperandSize(ins, opIdx);
                    else
                        read2Size = INS_MemoryOperandSize(ins, opIdx);

                    readOperandCount++;
                }
                if (INS_MemoryOperandIsWritten(ins, opIdx))
                {
                    writeSize = INS_MemoryOperandSize(ins, opIdx);
                    writeOperandCount++;
                }
            }

            if (readOperandCount > 0)
            {
                INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId, IARG_INST_PTR, offsetof(struct MEMREF, pc), IARG_MEMORYREAD_EA,
                                     offsetof(struct MEMREF, ea), IARG_UINT32, readSize, offsetof(struct MEMREF, size), IARG_BOOL,
                                     TRUE, offsetof(struct MEMREF, read), IARG_TSC, offsetof(struct MEMREF, tsc), IARG_END);
            }

            if (readOperandCount == 2)
            {
                INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId, IARG_INST_PTR, offsetof(struct MEMREF, pc), IARG_MEMORYREAD2_EA,
                                     offsetof(struct MEMREF, ea), IARG_UINT32, read2Size, offsetof(struct MEMREF, size),
                                     IARG_BOOL, TRUE, offsetof(struct MEMREF, read), IARG_TSC, offsetof(struct MEMREF, tsc),
                                     IARG_END);
            }

            if (writeOperandCount > 0)
            {
                INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId, IARG_INST_PTR, offsetof(struct MEMREF, pc), IARG_MEMORYWRITE_EA,
                                     offsetof(struct MEMREF, ea), IARG_UINT32, writeSize, offsetof(struct MEMREF, size),
                                     IARG_BOOL, FALSE, offsetof(struct MEMREF, read), IARG_TSC, offsetof(struct MEMREF, tsc),
                                     IARG_END);
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
    struct MEMREF* reference = (struct MEMREF*)buf;

    MLOG* mlog = static_cast< MLOG* >(PIN_GetThreadData(mlog_key, tid));

    mlog->DumpBufferToFile(reference, numElements, tid);

    return buf;
}

VOID ThreadStart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    // There is a new MLOG for every thread.  Opens the output file.
    MLOG* mlog = new MLOG;

    // A thread will need to look up its MLOG, so save pointer in TLS
    PIN_SetThreadData(mlog_key, mlog, tid);

    cerr << "Thread " << tid << " started" << endl << flush;
}

VOID ThreadFini(THREADID tid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    cerr << "Thread " << tid << " finished" << endl << flush;

    MLOG* mlog = static_cast< MLOG* >(PIN_GetThreadData(mlog_key, tid));

    delete mlog;

    PIN_SetThreadData(mlog_key, 0, tid);
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
    bufId = PIN_DefineTraceBuffer(sizeof(struct MEMREF), NUM_BUF_PAGES, BufferFull, 0);

    if (bufId == BUFFER_ID_INVALID)
    {
        cerr << "Error: could not allocate initial buffer" << endl;
        return 1;
    }

    // Initialize thread-specific data not handled by buffering api.
    mlog_key = PIN_CreateThreadDataKey(0);

    // add an instrumentation function
    TRACE_AddInstrumentFunction(Trace, 0);

    // add callbacks
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

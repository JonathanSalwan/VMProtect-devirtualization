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
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "pin.H"
using std::cerr;
using std::endl;
using std::hex;
using std::ofstream;
using std::string;

/*
 * Name of the output file
 */
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "buffer.out", "output file");

/*
 * Control of writing to the output file
 */
KNOB< BOOL > KnobDoWriteToOutputFile(KNOB_MODE_WRITEONCE, "pintool", "emit", "1", "control output to file");

/*
 * The ID of the buffer
 */
BUFFER_ID bufId;

/*
 * The lock for I/O.
 */
PIN_LOCK fileLock;

/*
 * There is an isolation bug in the Pin windows support that prevents
 * the pin tool from opening files ina callback routine.  If a tool
 * does this, deadlock occurs.  Instead, open one file in main, and
 * write the thread id along with the data.
 */
ofstream ofile;

/*
 * Number of OS pages for the buffer
 */
#define NUM_BUF_PAGES 1024

/*
 * Record of memory references.  Rather than having two separate
 * buffers for reads and writes, we just use one struct that includes a
 * flag for type.
 */
struct MEMREF
{
    THREADID tid;
    ADDRINT pc;
    ADDRINT ea;
    UINT32 size;
    UINT32 read;
};

/**************************************************************************
 *
 *  Instrumentation routines
 *
 **************************************************************************/

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
            UINT32 memoryOperands = INS_MemoryOperandCount(ins);

            for (UINT32 memOp = 0; memOp < memoryOperands; memOp++)
            {
                UINT32 refSize = INS_MemoryOperandSize(ins, memOp);

                // Note that if the operand is both read and written we log it once
                // for each.
                if (INS_MemoryOperandIsRead(ins, memOp))
                {
                    INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId, IARG_INST_PTR, offsetof(struct MEMREF, pc), IARG_MEMORYOP_EA,
                                         memOp, offsetof(struct MEMREF, ea), IARG_UINT32, refSize, offsetof(struct MEMREF, size),
                                         IARG_BOOL, TRUE, offsetof(struct MEMREF, read), IARG_END);
                }

                if (INS_MemoryOperandIsWritten(ins, memOp))
                {
                    INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId, IARG_INST_PTR, offsetof(struct MEMREF, pc), IARG_MEMORYOP_EA,
                                         memOp, offsetof(struct MEMREF, ea), IARG_UINT32, refSize, offsetof(struct MEMREF, size),
                                         IARG_BOOL, FALSE, offsetof(struct MEMREF, read), IARG_END);
                }
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
    /*
    This code will work - but it is very slow, so for testing purposes we run with the Knob turned off
    */
    if (KnobDoWriteToOutputFile)
    {
        PIN_GetLock(&fileLock, 1);

        struct MEMREF* reference = (struct MEMREF*)buf;

        for (UINT64 i = 0; i < numElements; i++, reference++)
        {
            if (reference->ea != 0) ofile << tid << "   " << reference->pc << "   " << reference->ea << endl;
        }
        PIN_ReleaseLock(&fileLock);
    }

    return buf;
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    PIN_GetLock(&fileLock, 1);
    ofile.close();
    PIN_ReleaseLock(&fileLock);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool demonstrates the basic use of the buffering API." << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
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

    // Initialize the memory reference buffer;
    // set up the callback to process the buffer.
    //
    bufId = PIN_DefineTraceBuffer(sizeof(struct MEMREF), NUM_BUF_PAGES, BufferFull, 0);

    if (bufId == BUFFER_ID_INVALID)
    {
        cerr << "Error: could not allocate initial buffer" << endl;
        return 1;
    }

    // Initialize the lock.
    PIN_InitLock(&fileLock);

    // Open the output file.
    string filename = KnobOutputFile.Value();
    ofile.open(filename.c_str());
    if (!ofile)
    {
        cerr << "Error: could not open output file." << endl;
        exit(1);
    }
    ofile << hex;

    // Add an instrumentation function
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

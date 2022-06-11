/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Sample buffering tool
 * 
 * This tool collects an address trace, including PC, read/write EA,
 * and read/write size, by filling a buffer.  When the buffer overflows,
 * the callback writes all of the collected records to a file.
 *
 */

#include "pin.H"
#include <iostream>
#include <stdio.h>
#include <stddef.h>
using std::cerr;
using std::endl;
using std::string;

/*
 * Name of the output file
 */
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "two_buffers.out", "output file");

/* Struct for holding memory references.  Rather than having two separate
 * buffers for loads and stores, we just use one struct that includes a
 * flag for type.
 */
struct MEMREF
{
    ADDRINT pc;
    ADDRINT address;
    UINT32 size;
    UINT32 load;
};

FILE* outfile;

BUFFER_ID bufId1, bufId2;
PIN_LOCK fileLock;

#define NUM_BUF_PAGES 1024

/*!
 *  Print out help message.
 */
INT32 Usage()
{
    cerr << "This tool demonstrates the basic use of the buffering API." << endl << endl;

    return -1;
}

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
                INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId1, IARG_INST_PTR, offsetof(struct MEMREF, pc), IARG_MEMORYREAD_EA,
                                     offsetof(struct MEMREF, address), IARG_UINT32, readSize, offsetof(struct MEMREF, size),
                                     IARG_UINT32, 1, offsetof(struct MEMREF, load), IARG_END);
            }
            if (readOperandCount == 2)
            {
                INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId1, IARG_INST_PTR, offsetof(struct MEMREF, pc), IARG_MEMORYREAD2_EA,
                                     offsetof(struct MEMREF, address), IARG_UINT32, read2Size, offsetof(struct MEMREF, size),
                                     IARG_UINT32, 1, offsetof(struct MEMREF, load), IARG_END);
            }
            if (writeOperandCount > 0)
            {
                INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId2, IARG_INST_PTR, offsetof(struct MEMREF, pc), IARG_MEMORYWRITE_EA,
                                     offsetof(struct MEMREF, address), IARG_UINT32, writeSize, offsetof(struct MEMREF, size),
                                     IARG_UINT32, 0, offsetof(struct MEMREF, load), IARG_END);
            }
        }
    }
}

/*!
 * Called when a buffer fills up, or the thread exits, so we can process it or pass it off
 * as we see fit.
 * @param[in] id		buffer handle
 * @param[in] tid		id of owning thread
 * @param[in] ctxt		application context when the buffer filled
 * @param[in] buf		actual pointer to buffer
 * @param[in] numElements	number of records
 * @param[in] v			callback value
 * @return  A pointer to the buffer to resume filling.
 */
VOID* BufferFull1(BUFFER_ID id, THREADID tid, const CONTEXT* ctxt, VOID* buf, UINT64 numElements, VOID* v)
{
    PIN_GetLock(&fileLock, 1);

    struct MEMREF* reference = (struct MEMREF*)buf;
    unsigned int i;

    fprintf(outfile, "Dumping buffer 1 at address %lx\n", (unsigned long)buf);

    for (i = 0; i < numElements; i++, reference++)
    {
        fprintf(outfile, "%lx %lx %u %u\n", (unsigned long)reference->pc, (unsigned long)reference->address, reference->size,
                reference->load);
    }
    fflush(outfile);
    PIN_ReleaseLock(&fileLock);

    return buf;
}

/*!
 * Called when a buffer fills up, or the thread exits, so we can process it or pass it off
 * as we see fit.
 * @param[in] id		buffer handle
 * @param[in] tid		id of owning thread
 * @param[in] ctxt		application context when the buffer filled
 * @param[in] buf		actual pointer to buffer
 * @param[in] numElements	number of records
 * @param[in] v			callback value
 * @return  A pointer to the buffer to resume filling.
 */
VOID* BufferFull2(BUFFER_ID id, THREADID tid, const CONTEXT* ctxt, VOID* buf, UINT64 numElements, VOID* v)
{
    PIN_GetLock(&fileLock, 1);

    struct MEMREF* reference = (struct MEMREF*)buf;
    unsigned int i;

    fprintf(outfile, "Dumping buffer 2 at address %lx\n", (unsigned long)buf);

    for (i = 0; i < numElements; i++, reference++)
    {
        fprintf(outfile, "%lx %lx %u %u\n", (unsigned long)reference->pc, (unsigned long)reference->address, reference->size,
                reference->load);
    }
    fflush(outfile);
    PIN_ReleaseLock(&fileLock);

    return buf;
}

/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the 
 *                              PIN_AddFiniFunction function call
 */
VOID Fini(INT32 code, VOID* v)
{
    PIN_GetLock(&fileLock, 1);
    fclose(outfile);
    printf("outfile closed\n");
    PIN_ReleaseLock(&fileLock);
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
    bufId1 = PIN_DefineTraceBuffer(sizeof(struct MEMREF), NUM_BUF_PAGES, BufferFull1, 0);
    if (bufId1 == BUFFER_ID_INVALID)
    {
        cerr << "Error allocating initial buffer 1" << endl;
        return 1;
    }

    // Initialize the memory reference buffer
    bufId2 = PIN_DefineTraceBuffer(sizeof(struct MEMREF), NUM_BUF_PAGES, BufferFull2, 0);
    if (bufId2 == BUFFER_ID_INVALID)
    {
        cerr << "Error allocating initial buffer 2" << endl;
        return 1;
    }

    outfile = fopen(KnobOutputFile.Value().c_str(), "w");
    if (!outfile)
    {
        cerr << "Couldn't open two_buffers.out" << endl;
        return 1;
    }

    PIN_InitLock(&fileLock);

    // add an instrumentation function
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register function to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

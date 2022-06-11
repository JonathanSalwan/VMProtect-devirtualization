/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::cout;
using std::endl;
using std::ostream;
using std::string;

ostream* OutFile = NULL;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "", "specify output file name");

INT32 numThreads          = 0;
const INT32 MaxNumThreads = 10000;

// The running count of instructions is kept here
// We let each thread's count be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
// This has shown to improve the performance of inscount2_mt by up to 6X
// on SPECOMP2001.
#define PADSIZE 56 // 64byte linesize : 64 - 8
struct THREAD_DATA
{
    UINT64 _count;
    UINT8 _pad[PADSIZE];
    THREAD_DATA() : _count(0) {}
};

// key for accessing TLS storage in the threads. initialized once in main()
static TLS_KEY tls_key;

// function to access thread-specific data
THREAD_DATA* get_tls(THREADID threadid)
{
    THREAD_DATA* tdata = static_cast< THREAD_DATA* >(PIN_GetThreadData(tls_key, threadid));
    return tdata;
}

// This function is called before every block
VOID PIN_FAST_ANALYSIS_CALL docount(ADDRINT c, THREADID tid)
{
    THREAD_DATA* tdata = get_tls(tid);
    tdata->_count += c;
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    numThreads++;
    ASSERT(numThreads <= MaxNumThreads, "Maximum number of threads exceeded\n");
    THREAD_DATA* tdata = new THREAD_DATA();
    PIN_SetThreadData(tls_key, tdata, threadid);
}

// Pin calls this function every time a new basic block is encountered
// It inserts a call to docount
VOID Trace(TRACE trace, VOID* v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to docount for every bbl, passing the number of instructions.
        // IPOINT_ANYWHERE allows Pin to schedule the call anywhere in the bbl to obtain best performance.

        BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR)docount, IARG_FAST_ANALYSIS_CALL, IARG_UINT32, BBL_NumIns(bbl),
                       IARG_THREAD_ID, IARG_END);
    }
}

// This function is called when the thread exits
VOID ThreadFini(THREADID threadIndex, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    THREAD_DATA* tdata = get_tls(threadIndex);
    *OutFile << "Count[" << decstr(threadIndex) << "] = " << tdata->_count << endl;
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v) { *OutFile << "Number of threads ever exist = " << numThreads << endl; }

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This Pintool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return 1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile = KnobOutputFile.Value().empty() ? &cout : new std::ofstream(KnobOutputFile.Value().c_str());

    // Obtain  a key for TLS storage.
    tls_key = PIN_CreateThreadDataKey(NULL);
    if (-1 == tls_key)
    {
        printf("number of already allocated keys reached the MAX_CLIENT_TLS_KEYS limit\n");
        PIN_ExitProcess(1);
    }

    PIN_AddThreadStartFunction(ThreadStart, NULL);

    // Register Fini to be called when thread exits.
    PIN_AddThreadFiniFunction(ThreadFini, NULL);

    // Register Fini to be called when the application exits.
    PIN_AddFiniFunction(Fini, NULL);

    // Register Instruction to be called to instrument instructions
    TRACE_AddInstrumentFunction(Trace, NULL);

    // Start the program, never returns
    PIN_StartProgram();

    return 1;
}

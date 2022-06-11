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

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "", "specify output file name");

INT32 numThreads = 0;
ostream* OutFile = NULL;

// Force each thread's data to be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
// This avoids the false sharing problem.
#define PADSIZE 56 // 64 byte line size: 64-8

// a running count of the instructions
class thread_data_t
{
  public:
    thread_data_t() : _count(0) {}
    UINT64 _count;
    UINT8 _pad[PADSIZE];
};

// key for accessing TLS storage in the threads. initialized once in main()
static TLS_KEY tls_key = INVALID_TLS_KEY;

// This function is called before every block
VOID PIN_FAST_ANALYSIS_CALL docount(UINT32 c, THREADID threadid)
{
    thread_data_t* tdata = static_cast< thread_data_t* >(PIN_GetThreadData(tls_key, threadid));
    tdata->_count += c;
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    numThreads++;
    thread_data_t* tdata = new thread_data_t;
    if (PIN_SetThreadData(tls_key, tdata, threadid) == FALSE)
    {
        cerr << "PIN_SetThreadData failed" << endl;
        PIN_ExitProcess(1);
    }
}

// Pin calls this function every time a new basic block is encountered.
// It inserts a call to docount.
VOID Trace(TRACE trace, VOID* v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to docount for every bbl, passing the number of instructions.

        BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR)docount, IARG_FAST_ANALYSIS_CALL, IARG_UINT32, BBL_NumIns(bbl),
                       IARG_THREAD_ID, IARG_END);
    }
}

// This function is called when the thread exits
VOID ThreadFini(THREADID threadIndex, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    thread_data_t* tdata = static_cast< thread_data_t* >(PIN_GetThreadData(tls_key, threadIndex));
    *OutFile << "Count[" << decstr(threadIndex) << "] = " << tdata->_count << endl;
    delete tdata;
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v) { *OutFile << "Total number of threads = " << numThreads << endl; }

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return 1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return Usage();

    OutFile = KnobOutputFile.Value().empty() ? &cout : new std::ofstream(KnobOutputFile.Value().c_str());

    // Obtain  a key for TLS storage.
    tls_key = PIN_CreateThreadDataKey(NULL);
    if (tls_key == INVALID_TLS_KEY)
    {
        cerr << "number of already allocated keys reached the MAX_CLIENT_TLS_KEYS limit" << endl;
        PIN_ExitProcess(1);
    }

    // Register ThreadStart to be called when a thread starts.
    PIN_AddThreadStartFunction(ThreadStart, NULL);

    // Register Fini to be called when thread exits.
    PIN_AddThreadFiniFunction(ThreadFini, NULL);

    // Register Fini to be called when the application exits.
    PIN_AddFiniFunction(Fini, NULL);

    // Register Instruction to be called to instrument instructions.
    TRACE_AddInstrumentFunction(Trace, NULL);

    // Start the program, never returns
    PIN_StartProgram();

    return 1;
}

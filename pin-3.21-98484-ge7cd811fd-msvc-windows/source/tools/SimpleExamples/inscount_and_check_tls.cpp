/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include <map>
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

// A map to store the data attached for each thread, and a lock to protect the map
std::map< THREADID, thread_data_t* > threads_data;
PIN_LOCK pinLock;

// function to access thread-specific data
thread_data_t* get_tls(THREADID threadid)
{
    thread_data_t* tdata = static_cast< thread_data_t* >(PIN_GetThreadData(tls_key, threadid));
    if (tdata == NULL)
    {
        cerr << "PIN_GetThreadData(" << tls_key << "," << threadid << ") Failed" << endl;
        PIN_ExitProcess(1);
    }

    PIN_GetLock(&pinLock, threadid);
    std::map< THREADID, thread_data_t* >::const_iterator it = threads_data.find(threadid);
    if ((it == threads_data.end()) || (it->second != tdata))
    {
        cerr << "PIN_GetThreadData(" << tls_key << "," << threadid << ") returned the wrong thread data" << endl;
        PIN_ExitProcess(1);
    }
    PIN_ReleaseLock(&pinLock);
    return tdata;
}

// This function is called before every block
VOID PIN_FAST_ANALYSIS_CALL docount(UINT32 c, THREADID threadid)
{
    thread_data_t* tdata = get_tls(threadid);
    tdata->_count += c;
}

// This function tests operations with illegal TLS keys.
VOID TestIllegalTLSOPerations(THREADID threadid)
{
    static bool tested = false;
    if (tested) return;
    tested = true;

    // start with an invalid key
    TLS_KEY temp_tls_key = INVALID_TLS_KEY;
    // Try to get thread data - expected to fail
    if (PIN_GetThreadData(temp_tls_key, threadid) != NULL)
    {
        cerr << "PIN_GetThreadData(" << tls_key << "," << threadid << ") should have failed" << endl;
        PIN_ExitProcess(1);
    }
    // Create a key
    temp_tls_key = PIN_CreateThreadDataKey(NULL);
    if (temp_tls_key == INVALID_TLS_KEY)
    {
        cerr << "number of already allocated keys reached the MAX_CLIENT_TLS_KEYS limit" << endl;
        PIN_ExitProcess(1);
    }
    // Delete the key
    if (PIN_DeleteThreadDataKey(temp_tls_key) == FALSE)
    {
        cerr << "PIN_DeleteThreadDataKey failed" << endl;
        PIN_ExitProcess(1);
    }
    // Delete it again - expected to fail
    if (PIN_DeleteThreadDataKey(temp_tls_key) == TRUE)
    {
        cerr << "PIN_DeleteThreadDataKey should have failed" << endl;
        PIN_ExitProcess(1);
    }
    // Try to use the deleted key - expected to fail
    if (PIN_SetThreadData(temp_tls_key, new thread_data_t, threadid) != FALSE)
    {
        cerr << "PIN_SetThreadData should have failed" << endl;
        PIN_ExitProcess(1);
    }
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
    PIN_GetLock(&pinLock, threadid + 1);
    threads_data.insert(std::make_pair(threadid, tdata));
    PIN_ReleaseLock(&pinLock);

    TestIllegalTLSOPerations(threadid);
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
    thread_data_t* tdata = get_tls(threadIndex);
    *OutFile << "Count[" << decstr(threadIndex) << "] = " << tdata->_count << endl;
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

    // Initialize the pin lock
    PIN_InitLock(&pinLock);

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

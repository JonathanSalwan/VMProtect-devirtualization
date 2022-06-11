/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  This file contains an ISA-portable PIN tool for tracing system calls
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

// key for accessing TLS storage in the threads. initialized once in main()
static TLS_KEY tls_key = INVALID_TLS_KEY;

// a running count of the instructions
union thread_data_t
{
    struct
    {
        UINT64 _lastSyscallStart;
        UINT64 _totalAccumulated;
    } _data;
    // Force each thread's data to be in its own data cache line so that
    // multiple threads do not contend for the same data cache line.
    // This avoids the false sharing problem.
    char _pad[64];
};

#ifdef TARGET_WINDOWS
extern "C" unsigned __int64 __rdtsc();
#pragma intrinsic(__rdtsc)
static inline UINT64 rdtsc() { return __rdtsc(); }
#else
static inline UINT64 rdtsc()
{
    UINT32 hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (((UINT64)lo) + (((UINT64)hi) << 32));
}
#endif

VOID SyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    thread_data_t* tdata = static_cast< thread_data_t* >(PIN_GetThreadData(tls_key, threadIndex));
    ASSERTX(0 == tdata->_data._lastSyscallStart);
    tdata->_data._lastSyscallStart = rdtsc();
}

VOID SyscallExit(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    thread_data_t* tdata           = static_cast< thread_data_t* >(PIN_GetThreadData(tls_key, threadIndex));
    tdata->_data._totalAccumulated = rdtsc() - tdata->_data._lastSyscallStart;
    tdata->_data._lastSyscallStart = 0;
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    numThreads++;
    thread_data_t* tdata = new thread_data_t;
    memset(tdata, 0, sizeof(*tdata));
    if (PIN_SetThreadData(tls_key, tdata, threadid) == FALSE)
    {
        cerr << "PIN_SetThreadData failed" << endl;
        PIN_ExitProcess(1);
    }
}

// This function is called when the thread exits
VOID ThreadFini(THREADID threadIndex, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    thread_data_t* tdata = static_cast< thread_data_t* >(PIN_GetThreadData(tls_key, threadIndex));
    *OutFile << "Thread " << decstr(threadIndex) << " spent " << tdata->_data._totalAccumulated
             << " processor cycles in system calls" << endl;
    delete tdata;
}

VOID Fini(INT32 code, VOID* v)
{
    if (OutFile != &cout) delete OutFile;
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This tool prints a log of system calls" + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
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

    PIN_AddSyscallEntryFunction(SyscallEntry, 0);

    PIN_AddSyscallExitFunction(SyscallExit, 0);

    PIN_AddFiniFunction(Fini, 0);

    PIN_CONFIGURATION_INFO configInfo = PIN_CreateDefaultConfigurationInfo();

    PIN_SetAllowedConcurrentCallbacks(configInfo, PIN_CALLBACK_TYPE_SYSCALL);

    // Start the program, never returns
    PIN_StartProgram(configInfo);

    return 0;
}

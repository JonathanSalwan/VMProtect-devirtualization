/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs
 *  and could serve as the starting point for developing your first PIN tool
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sched.h>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
/* ================================================================== */
// Global variables
/* ================================================================== */

UINT64 threadStartCount  = 0; //total number of started threads, including main thread
UINT64 threadCreateCount = 0; //total number of create threads, including main thread
UINT64 threadFiniCount   = 0; //total number of finished threads, including main thread

std::ostream* out = &cerr;

#if defined(TARGET_IA32)
#define SC_clone 120
#else
#define SC_clone 56
#endif

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "", "specify file name for cpp11Threads output");

/*!
 * Increase counter of threads in the application.
 * This function is called for every thread created by the application when it is
 * about to start running (including the root thread).
 * @param[in]   threadIndex     ID assigned by PIN to the new thread
 * @param[in]   ctxt            initial register state for the new thread
 * @param[in]   flags           thread creation flags (OS specific)
 * @param[in]   v               value specified by the tool in the 
 *                              PIN_AddThreadStartFunction function call
 */
VOID ThreadStart(THREADID threadIndex, CONTEXT* ctxt, INT32 flags, VOID* v) { threadStartCount++; }

VOID ThreadFini(THREADID threadIndex, const CONTEXT* ctxt, INT32 flags, VOID* v) { threadFiniCount++; }

enum SYSCALL_SEMANTIC
{
    SCS_UNKNOWN,
    SCS_EXIT,
    SCS_THREADEXIT,
    SCS_THREADCREATE,
    SCS_EXCEPTIONRETURN /* Signal return on Linux, APC/Callback/Exception on Windows */
};

BOOL IsVForkOrFork(ADDRINT arg) { return !(arg & ADDRINT(CLONE_VM | CLONE_THREAD)); }

// Map system calls from pre-system call machine state to semantic.
// Slightly more general than just using the call number, because the semantic
// can depend on the arguments too.
SYSCALL_SEMANTIC GetSyscallSemantic(CONTEXT* ctx, SYSCALL_STANDARD std)
{
    SYSCALL_SEMANTIC res = SCS_UNKNOWN;

    switch (PIN_GetSyscallNumber(ctx, std))
    {
        case SC_clone:
            // Check if this is a fork or a thread creation.
            {
                ADDRINT arg0 = PIN_GetSyscallArgument(ctx, std, 0);
                if (IsVForkOrFork(arg0))
                    res = SCS_UNKNOWN;
                else
                    res = SCS_THREADCREATE;
                break;
            }

        default:
            res = SCS_UNKNOWN;
            break;
    }
    return res;
}

VOID SyscallEntry(THREADID pin_tid, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    SYSCALL_SEMANTIC s = GetSyscallSemantic(ctxt, std);
    if (s == SCS_THREADCREATE)
    {
        threadCreateCount++;
    }
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
    *out << "===============================================" << endl;
    *out << "cpp11Threads analysis results: " << endl;
    *out << "Number of threads started: " << threadStartCount << endl;
    *out << "Number of threads created: " << threadCreateCount << endl;
    *out << "Number of threads finish: " << threadFiniCount << endl;
    *out << "===============================================" << endl;
}

INT32 Usage()
{
    cerr << "This pin tool counts predicated instructions selected by the\n"
            "following filter options\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary() << endl;
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

    threadStartCount  = 0;
    threadCreateCount = 0;
    threadFiniCount   = 0;

    string fileName = KnobOutputFile.Value();

    if (!fileName.empty())
    {
        out = new std::ofstream(fileName.c_str());
    }

    // Register function to be called for every thread before it starts running
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);

    PIN_AddSyscallEntryFunction(SyscallEntry, NULL);

    // Register function to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    cerr << "===============================================" << endl;
    if (!KnobOutputFile.Value().empty())
    {
        cerr << "See file " << KnobOutputFile.Value() << " for analysis results" << endl;
    }
    cerr << "===============================================" << endl;

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

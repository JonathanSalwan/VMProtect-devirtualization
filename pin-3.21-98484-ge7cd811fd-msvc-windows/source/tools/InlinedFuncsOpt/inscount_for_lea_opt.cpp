/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include "pin.H"

using std::cerr;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;

ofstream OutFile;

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount1   = 0;
static UINT64 icount2   = 0;
static UINT64 icount3   = 0;
static UINT64 icount4   = 0;
static UINT64 icount5   = 0;
static UINT64 thenCount = 0;

VOID docount() { icount1++; }
extern "C" VOID UpdateIcountByAdd(UINT64* icount_ptr);
extern "C" VOID UpdateIcountByInc(UINT64* icount_ptr);
extern "C" VOID UpdateIcountByDecInc(UINT64* icount_ptr);
extern "C" VOID UpdateIcountBySub(UINT64* icount_ptr);
extern "C" ADDRINT IfFuncWithAddThatCannotBeChangedToLea(UINT64* icount_ptr);

VOID ThenFuncThatShouldNeverBeCalled() { thenCount++; }

THREADID myThread = INVALID_THREADID;

ADDRINT IfMyThread(THREADID threadId) { return threadId == myThread; }

ADDRINT myThreadIfFuncWithAddThatCannotBeChangedToLea(THREADID threadId)
{
    return threadId == myThread && IfFuncWithAddThatCannotBeChangedToLea(NULL);
}

VOID Instruction(INS ins, VOID* v)
{
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfMyThread, IARG_THREAD_ID, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfMyThread, IARG_THREAD_ID, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)UpdateIcountByAdd, IARG_PTR, &icount2, IARG_END);
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfMyThread, IARG_THREAD_ID, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)UpdateIcountByInc, IARG_PTR, &icount3, IARG_END);
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfMyThread, IARG_THREAD_ID, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)UpdateIcountByDecInc, IARG_PTR, &icount4, IARG_END);
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfMyThread, IARG_THREAD_ID, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)UpdateIcountBySub, IARG_PTR, &icount5, IARG_END);
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)myThreadIfFuncWithAddThatCannotBeChangedToLea, IARG_THREAD_ID, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)ThenFuncThatShouldNeverBeCalled, IARG_END);
}

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inscount_for_lea_opt.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    if (icount1 != icount2 || icount1 != icount3 || icount1 != icount4 || icount1 != icount5 || thenCount != 0)
    {
        // Write to a file since cout and cerr maybe closed by the application
        OutFile.open(KnobOutputFile.Value().c_str());
        OutFile.setf(ios::showbase);
        if (thenCount != 0)
        {
            OutFile << "****ERROR thenCount was expected to be 0  not: " << thenCount << endl;
        }
        else
        {
            OutFile << "Count1 " << icount1 << endl;
            OutFile << "Count2 " << icount2 << endl;
            OutFile << "Count3 " << icount3 << endl;
            OutFile << "Count4 " << icount3 << endl;
            OutFile << "Count5 " << icount3 << endl;
            OutFile << "***ERROR - mismatch in icounts " << endl;
        }
        OutFile.close();
        exit(1);
    }
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (myThread == INVALID_THREADID)
    {
        myThread = threadid;
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    PIN_AddThreadStartFunction(ThreadStart, NULL);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, NULL);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, NULL);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

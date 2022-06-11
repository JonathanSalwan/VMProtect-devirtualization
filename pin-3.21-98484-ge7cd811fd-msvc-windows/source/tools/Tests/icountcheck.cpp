/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <iostream>
#include "pin.H"
using std::endl;

#if 0
#define DBG_PRINT(a) (a)
#else
#define DBG_PRINT(a)
#endif

// The tool assumes single-threaded application.
// This may not be the case on Windows 10.
// We arbitrary choose single thread to profile.
THREADID myThread = INVALID_THREADID;

ADDRINT IfMyThread(THREADID threadId)
{
    // Profile only single thread at any time
    return threadId == myThread;
}

VOID ThreadStart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    // Determine single thread to profile.
    if (myThread == INVALID_THREADID) myThread = tid;
}

UINT64 count_ins     = 0;
UINT64 count_bbl_ins = 0;

VOID docount_ins(void* pc)
{
    count_ins++;
    DBG_PRINT(printf("Anal: docount_ins: %lld at pc %p\n", count_ins, pc));
}

VOID docount_bbl_ins(void* pc, INT32 icount)
{
    count_bbl_ins += icount;
    DBG_PRINT(printf("Anal: docount_bbl_ins(%d): %lld at pc %p\n", icount, count_bbl_ins, pc));
    if (count_ins != count_bbl_ins)
    {
        std::cerr << "mismatch: count_ins " << count_ins << " != count_bbl_ins " << count_bbl_ins << " at pc " << pc << endl;
        exit(1);
    }
}

VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        DBG_PRINT(printf("Inst: Sequence address %p\n", (CHAR*)(INS_Address(BBL_InsHead(bbl)))));
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            DBG_PRINT(printf("Inst:   %p\n", (CHAR*)(INS_Address(ins))));
            INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
            INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(docount_ins), IARG_INST_PTR, IARG_END);
        }

        INT32 icount = BBL_NumIns(bbl);
        DBG_PRINT(printf("Inst:     -> control flow change (bbl size %d)\n", icount));
        INS ins = BBL_InsTail(bbl);
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(docount_bbl_ins), IARG_INST_PTR, IARG_UINT32, icount, IARG_END);
    }
}

VOID Fini(INT32 code, VOID* v) { std::cerr << "Count (ins) " << count_ins << ",  (bbl_ins) " << count_bbl_ins << endl; }

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    TRACE_AddInstrumentFunction(Trace, 0);

    // Add callbacks
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "pin.H"
using std::endl;

VOID* lastInstPtr;
BOOL isPredictedTaken;
VOID* predictedInstPtr;
UINT64 icount  = 0;
UINT64 errors  = 0;
BOOL isSkipped = TRUE; // always skip checking the first inst

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

VOID CountError()
{
    errors++;
    if (errors > 100)
    {
        std::cerr << "Too many errors, giving up\n";
        exit(errors);
    }
}

VOID CheckFlow(VOID* instPtr, INT32 isTaken, VOID* fallthroughAddr, VOID* takenAddr, UINT32 stutters)
{
    isPredictedTaken = isTaken;

    icount++;

    //fprintf(stderr,"Current: %p isTaken %d fallthroughAddr %p takenAddr %p\n", instPtr, isTaken, fallthroughAddr, takenAddr);

    if (predictedInstPtr != instPtr && !isSkipped &&
        !(stutters && (instPtr == lastInstPtr))) // An instruction which stutters can stay at the same IP
    {
        fprintf(stderr, "From: %p predicted InstPtr %p, actual InstPtr %p\n", lastInstPtr, predictedInstPtr, instPtr);
        CountError();
    }

    isSkipped = FALSE;

    if (isTaken)
    {
        predictedInstPtr = takenAddr;
    }
    else
    {
        predictedInstPtr = fallthroughAddr;
    }

    lastInstPtr = instPtr;
}

VOID Taken()
{
    if (!isPredictedTaken)
    {
        fprintf(stderr, "%p taken but not predictedInstPtr\n", lastInstPtr);
        CountError();
    }
}

VOID Skip() { isSkipped = TRUE; }

TLS_KEY ea_tls_key;

VOID SaveEa(THREADID tid, VOID* ea) { PIN_SetThreadData(ea_tls_key, ea, tid); }

VOID CheckXlatAfter(THREADID tid, ADDRINT eax)
{
    VOID* ea     = PIN_GetThreadData(ea_tls_key, tid);
    int actual   = *(char*)&eax;
    int expected = *(char*)ea;
    if (expected != actual)
    {
        fprintf(stderr, "xlat actual %d expected %d\n", actual, expected);
        errors++;
    }
}

#if defined(TARGET_IA32)
VOID CheckXlat(INS ins)
{
    if (INS_Opcode(ins) != XED_ICLASS_XLAT) return;
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SaveEa), IARG_THREAD_ID, IARG_MEMORYREAD_EA, IARG_END);
    INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(CheckXlatAfter), IARG_THREAD_ID, IARG_REG_VALUE, REG_EAX, IARG_END);
}
#else
VOID CheckXlat(INS ins) {}
#endif

VOID Instruction(INS ins, VOID* v)
{
    CheckXlat(ins);

    if (INS_HasFallThrough(ins) && INS_IsControlFlow(ins))
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckFlow, IARG_INST_PTR, IARG_BRANCH_TAKEN, IARG_FALLTHROUGH_ADDR,
                           IARG_BRANCH_TARGET_ADDR, IARG_UINT32, INS_Stutters(ins), IARG_END);
    }
    if (!INS_HasFallThrough(ins) && INS_IsControlFlow(ins))
    {
        /*
         * In this case there is no FallThrough address.
         * It is simply handled by putting 0 as the fallthroughAddr (the actual value does not matter,
         * because this control flow instruction is always taken).
         */
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckFlow, IARG_INST_PTR, IARG_BRANCH_TAKEN, IARG_ADDRINT, 0,
                           IARG_BRANCH_TARGET_ADDR, IARG_UINT32, INS_Stutters(ins), IARG_END);
    }
    if (INS_HasFallThrough(ins) && !INS_IsControlFlow(ins))
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckFlow, IARG_INST_PTR, IARG_BRANCH_TAKEN, IARG_FALLTHROUGH_ADDR,
                           IARG_ADDRINT, 0, IARG_UINT32, INS_Stutters(ins), IARG_END);
    }
    if (!INS_HasFallThrough(ins) && !INS_IsControlFlow(ins))
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckFlow, IARG_INST_PTR, IARG_BRANCH_TAKEN, IARG_ADDRINT, 0,
                           IARG_ADDRINT, 0, IARG_UINT32, INS_Stutters(ins), IARG_END);
    }

    if (INS_IsValidForIpointTakenBranch(ins))
    {
        INS_InsertIfCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)Taken, IARG_END);
    }

#if defined(TARGET_IA32) || defined(TARGET_IA32E)
    if (INS_IsSysenter(ins))
    { // sysenter on x86 has some funny control flow that we can't correctly verify for now
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)Skip, IARG_END);
    }
#endif
}

VOID SyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    // Profile only single thread
    if (myThread != threadIndex) return;

    Skip();
}

VOID Fini(INT32 code, VOID* v)
{
    if (code)
    {
        exit(code);
    }

    std::cerr << errors << " errors (" << icount << " instructions checked)" << endl;
    exit(errors);
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);
    // Use symbols to test handling of RTN of size 200000
    PIN_InitSymbols();

    ea_tls_key = PIN_CreateThreadDataKey(0);

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddSyscallEntryFunction(SyscallEntry, 0);

    // Add callbacks
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

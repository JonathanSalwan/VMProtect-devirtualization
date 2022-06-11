/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include "pin.H"

// unmatched_syscall_ip keeps track of the IP of the last syscall encounted.
// It is set before the syscall and reset afterward. Therefore, if the
// afterward instrumentation is bypassed, which could happen due to the funny
// control flow in sysenter on x86, then we will see a non-zero value in
// unmatched_syscall_ip when we execute the next syscall.

ADDRINT unmatched_syscall_ip = 0;

THREADID myThread = INVALID_THREADID;

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (myThread == INVALID_THREADID)
    {
        myThread = threadid;
    }
}

VOID BeforeSyscall(ADDRINT ip)
{
    if (unmatched_syscall_ip != 0)
    {
        printf("AfterSyscall() is not executed after the syscall at %lx\n", (unsigned long)unmatched_syscall_ip);
        fflush(stdout);
        exit(1);
    }

    unmatched_syscall_ip = ip;
}

VOID AfterSyscall(ADDRINT ip) { unmatched_syscall_ip = 0; }

VOID SyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    if (threadIndex == myThread)
    {
        BeforeSyscall(PIN_GetContextReg(ctxt, REG_INST_PTR));
    }
}

VOID SyscallExit(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    if (threadIndex == myThread)
    {
        AfterSyscall(PIN_GetContextReg(ctxt, REG_INST_PTR));
    }
}

VOID Instruction(INS ins, VOID* v)
{
    // For O/S's (macOS*) that don't support PIN_AddSyscallEntryFunction(),
    // instrument the system call instruction.

    if (INS_IsSyscall(ins) && INS_IsValidForIpointAfter(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)BeforeSyscall, IARG_INST_PTR, IARG_END);
        INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)AfterSyscall, IARG_INST_PTR, IARG_END);
    }
}

static VOID OnContextChange(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT* ctxtFrom, CONTEXT* ctxtTo,
                            INT32 info, VOID* v)
{
    // We may jump to UPC/signal while in system call (i.e. before it is return)
    // The code of this UPC may in its turn call another system call which will
    // see unmatched_syscall_ip != 0.
    unmatched_syscall_ip = 0;
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    PIN_AddThreadStartFunction(ThreadStart, NULL);

    INS_AddInstrumentFunction(Instruction, NULL);

    PIN_AddSyscallEntryFunction(SyscallEntry, NULL);
    PIN_AddSyscallExitFunction(SyscallExit, NULL);

    PIN_AddContextChangeFunction(OnContextChange, NULL);

    // Never returns
    PIN_StartProgram();

    return 0;
}

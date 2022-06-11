/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool is meant to be run on the application "async_multithreadingapp.c".  It attempts
 * to expose the following deadlock scenario.  Thread A enters an analysis
 * routine and acquires a tool lock.  Meanwhile thread B enters the VM
 * acquires the VM lock, and calls a call-back, which acquires the same tool
 * lock.  However, thread B blocks because thread A has the tool lock.  While
 * thread B is blocked, thread A recieves an asyncrhonous signal, and Pin's
 * handler attempts to acquire the VM lock causing deadlock.
 */

#include "pin.H"
#include <unistd.h>
#define USLEEP_TIME 50000

static VOID OnImage(IMG, VOID*);
static VOID OnSyscall(THREADID, CONTEXT*, SYSCALL_STANDARD, VOID*);
static VOID GetToolLock(THREADID);

PIN_LOCK ToolLock;

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();
    PIN_InitLock(&ToolLock);

    IMG_AddInstrumentFunction(OnImage, 0);
    PIN_AddSyscallEntryFunction(OnSyscall, 0);

    PIN_StartProgram();
    return 0;
}

static VOID OnImage(IMG img, VOID*)
{
    RTN rtn = RTN_FindByName(img, "DoToolAnalysis");
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(GetToolLock), IARG_THREAD_ID, IARG_END);
        RTN_Close(rtn);
    }
}

static VOID OnSyscall(THREADID tid, CONTEXT*, SYSCALL_STANDARD, VOID*)
{
    PIN_GetLock(&ToolLock, tid + 1);
    PIN_ReleaseLock(&ToolLock);
}

static VOID GetToolLock(THREADID tid)
{
    PIN_GetLock(&ToolLock, tid + 1);

    // The loop opens the timing hole, making the deadlock more likely.
    for (int i = 0; i < 10000000; i++)
        ;
    PIN_ReleaseLock(&ToolLock);
}

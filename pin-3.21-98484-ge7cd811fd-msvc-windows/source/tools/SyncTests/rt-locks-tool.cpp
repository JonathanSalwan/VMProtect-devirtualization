/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool is designed to run with the "rt-locks-app.cpp" application.  See
 * that source file for a description.
 */

#include <iostream>
#include <cstdlib>
#include "pin.H"
#include <sched.h>

static void InstrumentIns(INS, VOID*);
static void InstrumentRtn(RTN, VOID*);
static void OnExit(INT32, VOID*);
static void UseLocksWorker();
static void UseLocksScheduler();
static void CannotRunTest();

static BOOL FoundDoWork    = FALSE;
static BOOL FoundDoGetLock = FALSE;
static BOOL TestNotRun     = FALSE;

static PIN_LOCK Lock;
static unsigned long UseLockWorkerCount    = 0;
static unsigned long UseLockSchedulerCount = 0;

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();
    PIN_InitLock(&Lock);

    INS_AddInstrumentFunction(InstrumentIns, 0);
    RTN_AddInstrumentFunction(InstrumentRtn, 0);
    PIN_AddFiniFunction(OnExit, 0);
    PIN_StartProgram();
    return 0;
}

static void InstrumentIns(INS ins, VOID*)
{
    RTN rtn = INS_Rtn(ins);
    if (RTN_Valid(rtn) && RTN_Name(rtn) == "DoWorkInstrumentedWithPin")
    {
        FoundDoWork = true;
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(UseLocksWorker), IARG_END);
    }
}

static void InstrumentRtn(RTN rtn, VOID*)
{
    if (RTN_Name(rtn) == "DoGetLockWithPin")
    {
        FoundDoGetLock = true;
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(UseLocksScheduler), IARG_END);
        RTN_Close(rtn);
    }

    if (RTN_Name(rtn) == "TellPinNotSupported")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(CannotRunTest), IARG_END);
        RTN_Close(rtn);
    }
}

static void OnExit(INT32, VOID*)
{
    if (!TestNotRun && (!FoundDoWork || !FoundDoGetLock))
    {
        std::cout << "Couldn't find instrumentation routine(s)" << std::endl;
        exit(1);
    }
    std::cout << "Worker count for UseLocks   : " << std::dec << UseLockWorkerCount << std::endl;
    std::cout << "Scheduler count for UseLocks: " << std::dec << UseLockSchedulerCount << std::endl;
}

static void UseLocksWorker()
{
    UseLockWorkerCount++;

    // The worker thread executes this in a loop.  We do sched_yield() to
    // encourage the kernel to switch the thread off the CPU while it holds
    // the lock.  This would be possible even without the sched_yield(), though.
    //
    PIN_GetLock(&Lock, 1);
    sched_yield();
    PIN_ReleaseLock(&Lock);
}

static void UseLocksScheduler()
{
    UseLockSchedulerCount++;

    // The scheduler thread executes this after it has lowered the priority of
    // the worker thread.  The hope is that we will try to acquire the lock while
    // the worker already has it.
    //
    PIN_GetLock(&Lock, 2);
    PIN_ReleaseLock(&Lock);
}

static void CannotRunTest() { TestNotRun = TRUE; }

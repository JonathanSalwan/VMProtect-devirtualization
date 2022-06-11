/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sched.h>
using std::cerr;
using std::dec;
using std::hex;

/* Test PIN_SpawnThreadAndExecuteAt in a simple tool.
 * Can be run on any target application, since it only executes a few
 * instructions from there.
 */

static KNOB< BOOL > KnobVerbose(KNOB_MODE_WRITEONCE, "pintool", "v", "0", "Output verbose information");

static KNOB< BOOL > KnobFromCallback(KNOB_MODE_WRITEONCE, "pintool", "c", "0",
                                     "Create threads in callback, rather than analysis functions");

static UINT32 threadsCreated  = 0;
static ADDRINT threadFunction = 0;

// MAXTHREADS includes the static thread that starts the world, so must be >=2 to test anything!
#define MAXTHREADS 2
#define STACKSIZE 1024

static VOID cloneThread(CONTEXT* ctxt)
{
    // Should maybe use atomic increment here, but if we create a few bonus ones, it
    // doesn't really matter!
    UINT32 threadId = ++threadsCreated;

    if (threadId >= MAXTHREADS) return;

    CONTEXT localContext;

    if (!ctxt)
    {
        ctxt = &localContext;
        PIN_SetContextReg(ctxt, REG_GFLAGS, 0);
    }

    if (!PIN_SpawnApplicationThread(ctxt))
    {
        cerr << "PIN_SpawnApplicationThread failed\n";
        PIN_ExitProcess(-1);
    }
    if (KnobVerbose) cerr << "Spawned a new thread (" << threadId << ")\n";
}

static VOID ThreadCreateCallback(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (KnobVerbose) cerr << "Thread create callback for " << tid << "\n";

    // First thread is static, we don't want to mangle it, but we do create a new thread
    // from the callback.
    if (KnobFromCallback)
    {
        ADDRINT* stack = new ADDRINT[1024];
        CONTEXT context;

        ctxt = &context;

        if (!threadFunction)
        {
            cerr << "Cannot find 'doNothing()' in application\n";
            PIN_ExitProcess(-1);
        }

        // Fill in sensible values for critical registers.
        PIN_SetContextReg(ctxt, REG_STACK_PTR, ADDRINT(&stack[1023]));
        PIN_SetContextReg(ctxt, REG_INST_PTR, threadFunction);
        PIN_SetContextReg(ctxt, REG_GFLAGS, 0);

        cloneThread(ctxt);
    }

    if (tid >= (MAXTHREADS - 1))
    {
        cerr << "Created all threads OK\n";
        PIN_ExitProcess(0);
    }

    // First thread is created statically, we don't want to mess with it.
    if (tid == 0 || KnobFromCallback) return;

    if (!threadFunction)
    {
        cerr << "Cannot find 'doNothing()' in application\n";
        PIN_ExitProcess(-1);
    }

    ADDRINT* stack = new ADDRINT[1024];

    // Fill in sensible values for critical registers.
    PIN_SetContextReg(ctxt, REG_STACK_PTR, ADDRINT(&stack[1023]));
    PIN_SetContextReg(ctxt, REG_INST_PTR, ADDRINT(&threadFunction));
    PIN_SetContextReg(ctxt, REG_GFLAGS, 0);
}

static VOID ThreadExitCallback(THREADID tid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    cerr << "Thread " << tid << " terminated (" << code << ")\n";
    if (ctxt)
    {
        cerr << "Thread " << tid << " IP: " << hex << PIN_GetContextReg(ctxt, REG_INST_PTR) << dec << "\n";
    }
}

static VOID ContextChangeCallback(THREADID tid, CONTEXT_CHANGE_REASON reason, const CONTEXT* from, CONTEXT* to, INT32 info,
                                  VOID* v)
{
    cerr << "Thread " << tid << " context change " << reason << " info " << info << "\n";
    if (from)
    {
        cerr << "Thread " << tid << " IP: " << hex << PIN_GetContextReg(from, REG_INST_PTR) << dec << "\n";
    }
}

static VOID AddInstrumentation(INS ins, VOID*)
{
    static INT InstrumentedInstructions = 0;
    if (InstrumentedInstructions++ < MAXTHREADS)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)cloneThread, IARG_ADDRINT, 0, IARG_END);
    }
}

static VOID ImageLoadCallback(IMG img, VOID* v)
{
    if (threadFunction == 0)
    {
        if (KnobVerbose) cerr << "Looking for doNothing in " << IMG_Name(img) << "\n";
#ifndef TARGET_MAC
        RTN rtn = RTN_FindByName(img, "doNothing");
#else
        RTN rtn = RTN_FindByName(img, "_doNothing");
#endif

        if (RTN_Valid(rtn))
        {
            threadFunction = RTN_Address(rtn);
            if (KnobVerbose) cerr << "'doNothing' at " << hex << threadFunction << dec << "\n";
        }
    }
}

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        cerr << "Bad arguments\n";
        return -1;
    }

    PIN_InitSymbols();

    IMG_AddInstrumentFunction(ImageLoadCallback, 0);
    PIN_AddThreadStartFunction(ThreadCreateCallback, 0);
    PIN_AddThreadFiniFunction(ThreadExitCallback, 0);
    PIN_AddContextChangeFunction(ContextChangeCallback, 0);

    if (!KnobFromCallback) INS_AddInstrumentFunction(AddInstrumentation, 0);

    PIN_StartProgram();
}

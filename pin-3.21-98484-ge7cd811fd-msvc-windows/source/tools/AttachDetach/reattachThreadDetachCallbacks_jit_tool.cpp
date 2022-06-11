/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * JIT mode detach-reattach test which confirms that each thread get its respective thread detach callbacks.
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>
#include <sched.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
using std::endl;
using std::ofstream;
using std::string;

#define MAX_ITERATION 2

const char* FIRST_DLL_NAME = "my_dll.so";

const char* SECOND_DLL_NAME = "my_dll_1.so";

using std::cerr;
/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "reattachImageLoadCallbackAttached_jit.out",
                              "specify file name");

ofstream TraceFile;

PIN_LOCK pinLock;
/* ===================================================================== */

/* Session control checks that there is no callbacks mix between different
 * attach-detach iterations
 */
class SESSION_CONTROL
{
  public:
    SESSION_CONTROL() : _currentIteration(0), _threadCounter(0), _startAttachSession(FALSE), _startDetachSession(FALSE) {}

    static VOID AttachedThreadStart(THREADID threadIndex, CONTEXT* ctxt, VOID* arg);
    static VOID DedicatedThread(VOID* arg);

    VOID StartIteration(UINT32 it)
    {
        _currentIteration = it;
        _threadCounter    = 0;
    }
    UINT32 CurrentIteration() { return _currentIteration; }
    BOOL GotFirstThreadNotification(UINT32 it) { return ((it == _currentIteration) && (_threadCounter > 0)); }

    static SESSION_CONTROL* Instance() { return &m_instance; }

    VOID StartDetach()
    {
        _startDetachSession = TRUE;
        _startAttachSession = FALSE;
    }
    VOID StartAttach()
    {
        _startAttachSession = TRUE;
        _startDetachSession = FALSE;
    }
    VOID WaitForDetach()
    {
        while (!_startDetachSession)
            sched_yield();
    }
    VOID WaitForAttach()
    {
        while (!_startAttachSession)
            sched_yield();
    }

  private:
    UINT32 _currentIteration;
    UINT32 _threadCounter;
    volatile BOOL _startAttachSession;
    volatile BOOL _startDetachSession;
    static SESSION_CONTROL m_instance;
};

SESSION_CONTROL SESSION_CONTROL::m_instance;

SESSION_CONTROL* SessionControl() { return SESSION_CONTROL::Instance(); }

/* Detach session
 * Callbacks and function replacements
 */
class DETACH_SESSION
{
  public:
    // Detach completion notification
    static VOID DetachCompleted(VOID* v);
    static VOID DetachThreadStart(THREADID threadIndex, const CONTEXT* ctxt, VOID* arg);
    static VOID ImageLoad(IMG img, VOID* v);
};

/* Reattach session */
class REATTACH_SESSION
{
  public:
    static VOID ImageLoad(IMG img, VOID* v);
    static VOID AttachedThreadStart(THREADID threadIndex, CONTEXT* ctxt, VOID* arg);
};

/*
 * Pin-tool detach-completed callback
 * Called from Pin
 */
VOID DETACH_SESSION::DetachCompleted(VOID* v)
{
    unsigned long iteration = (unsigned long)v;
    if (iteration != SessionControl()->CurrentIteration())
    {
        cerr << "At " << __FUNCTION__ << "Iteration error: Expected " << SessionControl()->CurrentIteration() << " Received "
             << iteration << " In DetachThreadStart" << endl;
        PIN_ExitProcess(1);
    }

    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Detach session " << iteration << " Detach completed; tid = " << PIN_GetTid() << endl;
    if (iteration == MAX_ITERATION)
    {
        TraceFile << "TEST PASSED" << endl;
        TraceFile.close();
        PIN_ExitProcess(0);
    }
    PIN_ReleaseLock(&pinLock);
    SessionControl()->StartAttach();
}

BOOL afterAttachProbe(void) { return TRUE; }

VOID AttachMain(VOID* arg)
{
    UINT32 reattachIteration = *(reinterpret_cast< UINT32* >(&arg));
    SessionControl()->StartIteration(reattachIteration);
    IMG_AddInstrumentFunction(REATTACH_SESSION::ImageLoad, arg);
    PIN_AddDetachFunction(DETACH_SESSION::DetachCompleted, arg);
    PIN_AddThreadAttachFunction(SESSION_CONTROL::AttachedThreadStart, arg);
    PIN_AddThreadDetachFunction(DETACH_SESSION::DetachThreadStart, arg);
}

VOID SESSION_CONTROL::DedicatedThread(VOID* arg)
{
    ADDRINT reattachIteration = 2;
    while (1)
    {
        SessionControl()->WaitForDetach();
        PIN_GetLock(&pinLock, PIN_GetTid());
        TraceFile << "Pin tool: sending detach request" << endl;
        PIN_ReleaseLock(&pinLock);
        PIN_Detach();
        SessionControl()->WaitForAttach();
        PIN_GetLock(&pinLock, PIN_GetTid());
        TraceFile << "Pin tool: sending attach request" << endl;
        PIN_ReleaseLock(&pinLock);
        while (ATTACH_FAILED_DETACH == PIN_Attach(AttachMain, (VOID*)reattachIteration))
        {
            sched_yield();
        }
        reattachIteration++;
    }
}

/* Thread start notification in the first session */
VOID SESSION_CONTROL::AttachedThreadStart(THREADID threadIndex, CONTEXT* ctxt, VOID* arg)
{
    unsigned long iteration = (unsigned long)arg;
    if (iteration != SessionControl()->CurrentIteration())
    {
        cerr << "At " << __FUNCTION__ << "Iteration error: Expected " << SessionControl()->CurrentIteration() << " Received "
             << iteration << " In DetachThreadStart" << endl;
        PIN_ExitProcess(1);
    }
    PIN_GetLock(&pinLock, PIN_GetTid());
    ++(SessionControl()->_threadCounter);
    TraceFile << "Thread start "
              << " notification at session " << iteration << " tid " << PIN_GetTid() << endl;
    PIN_ReleaseLock(&pinLock);
}

VOID DETACH_SESSION::DetachThreadStart(THREADID threadIndex, const CONTEXT* ctxt, VOID* arg)
{
    unsigned long iteration = (unsigned long)arg;
    if (iteration != SessionControl()->CurrentIteration())
    {
        cerr << "At " << __FUNCTION__ << "Iteration error: Expected " << SessionControl()->CurrentIteration() << " Received "
             << iteration << " In DetachThreadStart" << endl;
        PIN_ExitProcess(1);
    }
    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Thread detach "
              << " notification at session " << iteration << " tid " << PIN_GetTid() << endl;
    PIN_ReleaseLock(&pinLock);
}

/*
 *Image load callback for the first Pin session
 */
VOID DETACH_SESSION::ImageLoad(IMG img, VOID* v)
{
    unsigned long iteration = (unsigned long)v;
    if (iteration != SessionControl()->CurrentIteration())
    {
        cerr << "At " << __FUNCTION__ << "Iteration error: Expected " << SessionControl()->CurrentIteration() << " Received "
             << iteration << " In DetachThreadStart" << endl;
        PIN_ExitProcess(1);
    }
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_FindByName(img, "AfterAttach1");

        // relevent only in the attach scenario.
        if (RTN_Valid(rtn))
        {
            RTN_Replace(rtn, AFUNPTR(afterAttachProbe));
        }
    }

    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Load image " << IMG_Name(img) << " in iteration " << iteration << endl;
    PIN_ReleaseLock(&pinLock);
    size_t found;
    found = IMG_Name(img).find(FIRST_DLL_NAME);
    if (found != string::npos)
    {
        SessionControl()->StartDetach();
    }
}

/*
 *Image load callback for the second Pin session
 */
VOID REATTACH_SESSION::ImageLoad(IMG img, VOID* v)
{
    unsigned long iteration = (unsigned long)v;
    if (iteration != SessionControl()->CurrentIteration())
    {
        cerr << "At " << __FUNCTION__ << "Iteration error: Expected " << SessionControl()->CurrentIteration() << " Received "
             << iteration << " In DetachThreadStart" << endl;
        PIN_ExitProcess(1);
    }
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_FindByName(img, "AfterAttach2");
        if (RTN_Valid(rtn))
        {
            RTN_Replace(rtn, AFUNPTR(afterAttachProbe));
        }
    }

    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Load image " << IMG_Name(img) << " in iteration " << iteration << endl;
    PIN_ReleaseLock(&pinLock);

    size_t found;
    found = IMG_Name(img).find(SECOND_DLL_NAME);
    if (found != string::npos)
    {
        SessionControl()->StartDetach();
    }
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);
    SessionControl()->StartIteration(1);
    PIN_InitLock(&pinLock);
    TraceFile.open(KnobOutputFile.Value().c_str());
    IMG_AddInstrumentFunction(DETACH_SESSION::ImageLoad, (VOID*)1);
    PIN_AddDetachFunction(DETACH_SESSION::DetachCompleted, (VOID*)1);
    PIN_AddThreadAttachFunction(SESSION_CONTROL::AttachedThreadStart, (VOID*)1);
    PIN_AddThreadDetachFunction(DETACH_SESSION::DetachThreadStart, (VOID*)1);

    THREADID tid = PIN_SpawnInternalThread(SESSION_CONTROL::DedicatedThread, NULL, 0x40000, NULL);
    assert(tid != INVALID_THREADID);
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

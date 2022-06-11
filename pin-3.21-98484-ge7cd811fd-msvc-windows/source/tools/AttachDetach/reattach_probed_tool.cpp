/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * Probe mode detach-reattach test
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>
#include <sched.h>
#include <unistd.h>
using std::endl;
using std::ofstream;
using std::string;

using std::cerr;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "reattach_probed_tool.out", "specify file name");

KNOB< BOOL > KnobUniqueTraceFile(KNOB_MODE_WRITEONCE, "pintool", "uniq", "0", "unique trace file name");

ofstream TraceFile;
PIN_LOCK pinLock;
/* ===================================================================== */

#define MAX_ITERATION 5

void BeforeFork(UINT32 childPid, void* data) { TraceFile << "TOOL: Before fork." << PIN_GetPid() << endl; }

void AfterForkInParent(UINT32 childPid, void* data) { TraceFile << "TOOL: After fork in parent." << PIN_GetPid() << endl; }

void AfterForkInChild(UINT32 childPid, void* data) { TraceFile << "TOOL: After fork in child." << PIN_GetPid() << endl; }

BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData) { return TRUE; }

/*
 * Main function for re-attach
 */
VOID AttachMain(VOID* arg);

/* Session control checks that there is no callbacks mix between different 
 * attach-detach iterations
 */
class SESSION_CONTROL
{
  public:
    SESSION_CONTROL() : _currentIteration(0), _threadCounter(0), _startAttachSession(FALSE), _startDetachSession(FALSE) {}

    static VOID ApplicationStart(VOID* v);
    static VOID AttachedThreadStart(VOID* sigmask, VOID* v);
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
    DETACH_SESSION() {}
    // Detach completion notification
    static VOID DetachCompleted(VOID* v);
    static VOID ImageLoad(IMG img, void* v);
    static VOID ImageUnload(IMG img, void* v);

    static DETACH_SESSION* Instance() { return &m_instance; }

  private:
    static DETACH_SESSION m_instance;
};

DETACH_SESSION DETACH_SESSION::m_instance;

DETACH_SESSION* DtSession() { return DETACH_SESSION::Instance(); }

/* Reattach session */
class REATTACH_SESSION
{
  public:
    REATTACH_SESSION() {}
    static VOID ImageLoad(IMG img, void* v);
    static VOID ImageUnload(IMG img, void* v);
    static VOID ApplicationStart(VOID* v);
    static VOID AttachedThreadStart(VOID* sigmask, VOID* v);
    static BOOL IsAttachCompleted();

    static REATTACH_SESSION* Instance() { return &m_instance; }

  private:
    static REATTACH_SESSION m_instance;
};

REATTACH_SESSION REATTACH_SESSION::m_instance;

REATTACH_SESSION* AtSession() { return REATTACH_SESSION::Instance(); }

/*
 * Pin-tool detach-completed callback
 * Called from Pin
 */
VOID DETACH_SESSION::DetachCompleted(VOID* arg)
{
    unsigned long detachIteration = (unsigned long)arg;
    if (detachIteration != SessionControl()->CurrentIteration())
    {
        cerr << "Detach iteration error: Expected " << SessionControl()->CurrentIteration() << " Rececived " << detachIteration
             << " In DetachCompleted" << endl;
        exit(-1);
    }
    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Detach session " << detachIteration << " Detach completed; tid = " << PIN_GetTid() << endl;

    fprintf(stderr, "Iteration %lu completed\n", detachIteration);
    //cerr << "Iteration " << detachIteration << " completed." << endl;
    if (detachIteration >= MAX_ITERATION)
    {
        TraceFile << "TEST PASSED" << endl;
        TraceFile.close();
        exit(0);
    }
    PIN_ReleaseLock(&pinLock);

    sleep(1);
    SessionControl()->StartAttach();
}

/* 
 *Image load callback for the first Pin session
 */
VOID DETACH_SESSION::ImageLoad(IMG img, void* arg)
{
    unsigned long detachIteration = (unsigned long)arg;
    if (detachIteration != SessionControl()->CurrentIteration())
    {
        cerr << "Detach iteration error: Expected " << SessionControl()->CurrentIteration() << " Received " << detachIteration
             << " In ImageLoad" << endl;
        exit(-1);
    }
    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Load image " << IMG_Name(img) << endl;
    PIN_ReleaseLock(&pinLock);
}
VOID DETACH_SESSION::ImageUnload(IMG img, void* arg)
{
    unsigned long detachIteration = (unsigned long)arg;
    if (detachIteration != SessionControl()->CurrentIteration())
    {
        cerr << "Detach iteration error: Expected " << SessionControl()->CurrentIteration() << " Received " << detachIteration
             << " In ImageUnload" << endl;
        exit(-1);
    }
}
/* Application start notification in the first session */
VOID SESSION_CONTROL::ApplicationStart(VOID* arg)
{
    unsigned long iteration = (unsigned long)arg;
    if (iteration != SessionControl()->CurrentIteration())
    {
        cerr << "Iteration error: Expected " << SessionControl()->CurrentIteration() << " Received " << iteration
             << " In ApplicationStart" << endl;
        exit(-1);
    }

    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Application start notification at session " << iteration << endl;
    PIN_ReleaseLock(&pinLock);

    sleep(1);
    SessionControl()->StartDetach();
}

/* Thread start notification in the first session */
VOID SESSION_CONTROL::AttachedThreadStart(VOID* sigmask, VOID* arg)
{
    unsigned long iteration = (unsigned long)arg;
    if (iteration != SessionControl()->CurrentIteration())
    {
        cerr << "Iteration error: Expected " << SessionControl()->CurrentIteration() << " Received " << iteration
             << " In AttachedThreadStart" << endl;
        exit(-1);
    }
    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Thread start " << ++(SessionControl()->_threadCounter) << " notification at session " << iteration << " tid "
              << PIN_GetTid() << endl;
    PIN_ReleaseLock(&pinLock);
}

VOID SESSION_CONTROL::DedicatedThread(VOID* arg)
{
    static ADDRINT reattachIteration = 2;
    while (1)
    {
        SessionControl()->WaitForDetach();
        PIN_GetLock(&pinLock, PIN_GetTid());
        TraceFile << "Pin tool: sending detach request" << endl;
        PIN_ReleaseLock(&pinLock);
        PIN_DetachProbed();

        SessionControl()->WaitForAttach();
        PIN_GetLock(&pinLock, PIN_GetTid());
        TraceFile << "Pin tool: sending attach request" << endl;
        PIN_ReleaseLock(&pinLock);
        PIN_AttachProbed(AttachMain, (VOID*)reattachIteration++);
    }
}

/* 
 *Image load callback for the second Pin session
 */
VOID REATTACH_SESSION::ImageLoad(IMG img, void* arg)
{
    unsigned long reattachIteration = (unsigned long)arg;
    if (reattachIteration != SessionControl()->CurrentIteration())
    {
        cerr << "Iteration error: Expected " << SessionControl()->CurrentIteration() << " Received " << reattachIteration
             << " In ImageLoad" << endl;
        exit(-1);
    }
}

VOID REATTACH_SESSION::ImageUnload(IMG img, void* arg)
{
    unsigned long reattachIteration = (unsigned long)arg;
    if (reattachIteration != SessionControl()->CurrentIteration())
    {
        cerr << "Iteration error: Expected " << SessionControl()->CurrentIteration() << " Received " << reattachIteration
             << " In ImageUnload" << endl;
        exit(-1);
    }
}

/* Return TRUE if the tool is notified about app start */
BOOL REATTACH_SESSION::IsAttachCompleted() { return SessionControl()->GotFirstThreadNotification(2); }

VOID AttachMain(VOID* arg)
{
    UINT32 reattachIteration = *(reinterpret_cast< UINT32* >(&arg));
    SessionControl()->StartIteration(reattachIteration);

    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Re-attach session start, inside AttachMain; iteration " << reattachIteration << endl;
    PIN_ReleaseLock(&pinLock);

    IMG_AddInstrumentFunction(REATTACH_SESSION::ImageLoad, arg);
    IMG_AddUnloadFunction(REATTACH_SESSION::ImageUnload, arg);

    PIN_AddApplicationStartFunction(SESSION_CONTROL::ApplicationStart, arg);
    PIN_AddThreadAttachProbedFunction(SESSION_CONTROL::AttachedThreadStart, arg);
    PIN_AddDetachFunctionProbed(DETACH_SESSION::DetachCompleted, arg);
    PIN_AddForkFunctionProbed(FPOINT_BEFORE, BeforeFork, 0);
    PIN_AddForkFunctionProbed(FPOINT_AFTER_IN_CHILD, AfterForkInChild, 0);
    PIN_AddForkFunctionProbed(FPOINT_AFTER_IN_PARENT, AfterForkInParent, 0);
    PIN_AddFollowChildProcessFunction(FollowChild, 0);
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    PIN_InitLock(&pinLock);

    char pidStr[10];
    pidStr[0] = '\0';
    if (KnobUniqueTraceFile.Value())
    {
        sprintf(pidStr, "%d", PIN_GetPid());
    }
    string traceFileName = KnobOutputFile.Value() + pidStr;

    TraceFile.open(traceFileName.c_str());

    SessionControl()->StartIteration(1);
    IMG_AddInstrumentFunction(DETACH_SESSION::ImageLoad, (VOID*)1);
    IMG_AddUnloadFunction(DETACH_SESSION::ImageUnload, (VOID*)1);
    PIN_AddDetachFunctionProbed(DETACH_SESSION::DetachCompleted, (VOID*)1);
    PIN_AddApplicationStartFunction(SESSION_CONTROL::ApplicationStart, (VOID*)1);
    PIN_AddThreadAttachProbedFunction(SESSION_CONTROL::AttachedThreadStart, (VOID*)1);
    PIN_AddForkFunctionProbed(FPOINT_BEFORE, BeforeFork, 0);
    PIN_AddForkFunctionProbed(FPOINT_AFTER_IN_CHILD, AfterForkInChild, 0);
    PIN_AddForkFunctionProbed(FPOINT_AFTER_IN_PARENT, AfterForkInParent, 0);
    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    THREADID tid = PIN_SpawnInternalThread(SESSION_CONTROL::DedicatedThread, NULL, 0x40000, NULL);
    ASSERTX(tid != INVALID_THREADID);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

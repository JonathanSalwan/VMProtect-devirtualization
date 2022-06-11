/*
 * Copyright (C) 2012-2021 Intel Corporation.
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
#include <linux/unistd.h>
#include <sys/syscall.h>
using std::endl;
using std::ofstream;
using std::string;

#define MAX_ITERATION 2

const char* FIRST_DLL_NAME = "my_dll.so";

const char* SECOND_DLL_NAME = "my_dll_1.so";

static int iteration = 1;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "reattachImageLoadCallbackAttached.out", "specify file name");

ofstream TraceFile;

PIN_LOCK pinLock;
/* ===================================================================== */

/* Session control checks that there is no callbacks mix between different
 * attach-detach iterations
 */
class SESSION_CONTROL
{
  public:
    SESSION_CONTROL() : _startAttachSession(FALSE), _startDetachSession(FALSE) {}

    static INT DedicatedThread(VOID* arg);

    static SESSION_CONTROL* Instance();

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
    volatile BOOL _startAttachSession;
    volatile BOOL _startDetachSession;
    static SESSION_CONTROL* m_instance;
};

SESSION_CONTROL* SESSION_CONTROL::m_instance = NULL;

SESSION_CONTROL* SESSION_CONTROL::Instance()
{
    if (!m_instance) m_instance = new SESSION_CONTROL;
    return m_instance;
}

SESSION_CONTROL* SessionControl() { return SESSION_CONTROL::Instance(); }

/* Detach session
 * Callbacks and function replacements
 */
class DETACH_SESSION
{
  public:
    // Detach completion notification
    static VOID DetachCompleted(VOID* v);
    static VOID ImageLoad(IMG img, VOID* v);
};

/* Reattach session */
class REATTACH_SESSION
{
  public:
    static VOID ImageLoad(IMG img, VOID* v);
};

/*
 * Pin-tool detach-completed callback
 * Called from Pin
 */
VOID DETACH_SESSION::DetachCompleted(VOID* v)
{
    if (iteration == MAX_ITERATION)
    {
        PIN_ExitProcess(0);
    }

    SessionControl()->StartAttach();
}

BOOL afterAttachProbe(void) { return TRUE; }

/*
 *Image load callback for the first Pin session
 */
VOID DETACH_SESSION::ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_FindByName(img, "AfterAttach1");

        // relevent only in the attach scenario.
        if (RTN_Valid(rtn))
        {
            RTN_ReplaceProbed(rtn, AFUNPTR(afterAttachProbe));
        }
    }

    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Load image " << IMG_Name(img) << "in iteration " << iteration << endl;
    PIN_ReleaseLock(&pinLock);
    size_t found;
    found = IMG_Name(img).find(FIRST_DLL_NAME);
    if (found != string::npos)
    {
        SessionControl()->StartDetach();
    }
}

VOID AttachMain(VOID* arg)
{
    IMG_AddInstrumentFunction(REATTACH_SESSION::ImageLoad, 0);
    iteration++;
    PIN_AddDetachFunctionProbed(DETACH_SESSION::DetachCompleted, 0);
}

INT SESSION_CONTROL::DedicatedThread(VOID* arg)
{
    while (1)
    {
        SessionControl()->WaitForDetach();
        PIN_DetachProbed();
        SessionControl()->WaitForAttach();
        PIN_AttachProbed(AttachMain, 0);
    }
    return 0;
}

/*
 *Image load callback for the second Pin session
 */
VOID REATTACH_SESSION::ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_FindByName(img, "AfterAttach2");
        if (RTN_Valid(rtn))
        {
            RTN_ReplaceProbed(rtn, AFUNPTR(afterAttachProbe));
        }
    }

    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Load image " << IMG_Name(img) << " in iteration" << iteration << endl;
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

    PIN_InitLock(&pinLock);
    TraceFile.open(KnobOutputFile.Value().c_str());
    IMG_AddInstrumentFunction(DETACH_SESSION::ImageLoad, 0);
    PIN_AddDetachFunctionProbed(DETACH_SESSION::DetachCompleted, 0);

    UINT32 stackSize = 0x40000;
    UINT8* stack     = new UINT8[stackSize];

    // thread stack should be aligned
    UINT8* stackTop = (UINT8*)((((ADDRINT)stack + stackSize) >> 4) << 4);
    int flags       = (CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_PARENT);
    int tid         = clone(SESSION_CONTROL::DedicatedThread, stackTop, flags, 0);
    assert(tid != -1);
    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

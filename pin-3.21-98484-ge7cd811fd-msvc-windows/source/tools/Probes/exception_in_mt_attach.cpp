/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sched.h>
#include <assert.h>
using std::cerr;
using std::dec;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "probe_tool.out", "specify file name");

ofstream TraceFile;
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool tests MT attach in probe mode.\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

PIN_LOCK pinLock;

UINT32 threadCounter    = 0;
BOOL isAppStartReceived = FALSE;

VOID AppStart(VOID* v)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Application Start Callback is called from thread " << dec << PIN_GetTid() << endl;
    isAppStartReceived = TRUE;
    PIN_ReleaseLock(&pinLock);
}

VOID AttachedThreadStart(VOID* sigmask, VOID* v)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Thread counter is updated to " << dec << (threadCounter + 1) << endl;
    ++threadCounter;
    PIN_ReleaseLock(&pinLock);
}

int PinReady(unsigned int numOfThreads)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    // Check that we don't have any extra thread
    assert(threadCounter <= numOfThreads);
    if ((threadCounter == numOfThreads) && isAppStartReceived)
    {
        TraceFile.close();
        PIN_ReleaseLock(&pinLock);
        return 1;
    }
    PIN_ReleaseLock(&pinLock);
    return 0;
}

typedef int (*foo_t)();

static int foo_rep(foo_t orig_foo, ADDRINT returnIp)
{
    printf("foo rep called\n");

    int res = orig_foo();

    // May not be executed if exception occurs in previous statement
    printf("Caller IP = %s\n", hexstr(returnIp).c_str());

    return res;
}

VOID ImageLoad(IMG img, void* v)
{
    RTN rtn = RTN_FindByName(img, "ThreadsReady");
    if (RTN_Valid(rtn))
    {
        RTN_ReplaceProbed(rtn, AFUNPTR(PinReady));
    }

    if (IMG_IsMainExecutable(img))
    {
        RTN routine = RTN_FindByName(img, "foo");
        if (RTN_Valid(routine))
        {
            PROTO foo_proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "foo", PIN_PARG_END());
            AFUNPTR foo_ptr = RTN_ReplaceSignatureProbed(routine, (AFUNPTR)foo_rep, IARG_PROTOTYPE, foo_proto, IARG_ORIG_FUNCPTR,
                                                         IARG_RETURN_IP, IARG_END);
            ASSERTX(foo_ptr != 0);
        }
    }
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    TraceFile.open(KnobOutputFile.Value().c_str());
    TraceFile << hex;
    TraceFile.setf(ios::showbase);

    PIN_InitLock(&pinLock);

    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddApplicationStartFunction(AppStart, 0);
    PIN_AddThreadAttachProbedFunction(AttachedThreadStart, 0);
    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

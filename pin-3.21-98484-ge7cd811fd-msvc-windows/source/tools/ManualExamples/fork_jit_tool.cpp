/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * A test for callbacks around fork in jit mode.
 */
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include "pin.H"

#include <iostream>
#include <fstream>
using std::cerr;
using std::endl;

INT32 Usage()
{
    cerr << "This pin tool registers callbacks around fork().\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

pid_t parent_pid;
PIN_LOCK pinLock;

VOID BeforeFork(THREADID threadid, const CONTEXT* ctxt, VOID* arg)
{
    PIN_GetLock(&pinLock, threadid + 1);
    cerr << "TOOL: Before fork." << endl;
    PIN_ReleaseLock(&pinLock);
    parent_pid = PIN_GetPid();
}

VOID AfterForkInParent(THREADID threadid, const CONTEXT* ctxt, VOID* arg)
{
    PIN_GetLock(&pinLock, threadid + 1);
    cerr << "TOOL: After fork in parent." << endl;
    PIN_ReleaseLock(&pinLock);

    if (PIN_GetPid() != parent_pid)
    {
        cerr << "PIN_GetPid() fails in parent process" << endl;
        exit(-1);
    }
}

VOID AfterForkInChild(THREADID threadid, const CONTEXT* ctxt, VOID* arg)
{
    PIN_GetLock(&pinLock, threadid + 1);
    cerr << "TOOL: After fork in child." << endl;
    PIN_ReleaseLock(&pinLock);

    if ((PIN_GetPid() == parent_pid) || (getppid() != parent_pid))
    {
        cerr << "PIN_GetPid() fails in child process" << endl;
        exit(-1);
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    // Initialize the pin lock
    PIN_InitLock(&pinLock);

    // Register a notification handler that is called when the application
    // forks a new process.
    PIN_AddForkFunction(FPOINT_BEFORE, BeforeFork, 0);
    PIN_AddForkFunction(FPOINT_AFTER_IN_PARENT, AfterForkInParent, 0);
    PIN_AddForkFunction(FPOINT_AFTER_IN_CHILD, AfterForkInChild, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

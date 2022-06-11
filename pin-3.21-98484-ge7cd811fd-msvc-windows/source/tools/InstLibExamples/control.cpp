/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>

#include "pin.H"
#include "instlib.H"
#include "control_manager.H"

using namespace INSTLIB;
using namespace CONTROLLER;

#if defined(__GNUC__)
#if defined(__APPLE__)
#define ALIGN_LOCK __attribute__((aligned(16))) /* apple only supports 16B alignment */
#else
#define ALIGN_LOCK __attribute__((aligned(64)))
#endif
#else
#define ALIGN_LOCK __declspec(align(64))
#endif

static PIN_LOCK ALIGN_LOCK output_lock;

// Track the number of instructions executed
ICOUNT icount;

// Contains knobs and instrumentation to recognize start/stop points
CONTROL_MANAGER control("controller_");

VOID Handler(EVENT_TYPE ev, VOID* v, CONTEXT* ctxt, VOID* ip, THREADID tid, BOOL bcast)
{
    PIN_GetLock(&output_lock, tid + 1);

    std::cout << "tid: " << tid << " ";
    std::cout << "ip: " << ip << " " << icount.Count();

    switch (ev)
    {
        case EVENT_START:
            std::cout << "Start" << endl;
            break;

        case EVENT_STOP:
            std::cout << "Stop" << endl;
            break;

        case EVENT_THREADID:
            std::cout << "ThreadID" << endl;
            break;

        default:
            ASSERTX(false);
            break;
    }
    PIN_ReleaseLock(&output_lock);
}

INT32 Usage()
{
    cerr << "This pin tool demonstrates use of CONTROL to identify start/stop points in a program\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    PIN_InitLock(&output_lock);
    icount.Activate();

    // Activate alarm, must be done before PIN_StartProgram
    control.RegisterHandler(Handler, 0, FALSE);
    control.Activate();

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

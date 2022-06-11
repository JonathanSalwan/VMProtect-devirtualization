/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"
#include <iostream>
#include "control_manager.H"

using namespace CONTROLLER;

//
// This test case combines detach.cpp and control.pp
// It calls PIN_Detach() whenever a specified region is reached.
// You can specify regions by "-skip, -start_address, -ppfile" etc.
//

// Contains knobs and instrumentation to recognize start/stop points
CONTROL_MANAGER control;

VOID Handler(EVENT_TYPE ev, VOID* v, CONTEXT* ctxt, VOID* ip, THREADID tid, BOOL bcast)
{
    switch (ev)
    {
        case EVENT_START:
            if (ip == 0)
            {
                std::cerr << " IP zero before detach; use -skip/-ppfile/-start_address to specify detach location." << endl;
                ASSERTX(false);
            }
            std::cerr << "Start : Detaching at IP: " << hex << ip << endl;
            PIN_Detach();
            break;

        case EVENT_STOP:
            std::cerr << "Stop" << endl;
            break;

        default:
            ASSERT(false, "Received control event " + decstr(ev) + "\n");
            break;
    }
}

VOID helloWorld(VOID* v) { fprintf(stdout, "Hello world!\n"); }

VOID byeWorld(VOID* v) { fprintf(stdout, "Byebye world!\n"); }

INT32 Usage()
{
    cerr << "This pin tool demonstrates uses CONTROL to identify start points in a program and does a PIN_Detach() at those "
            "points. \n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    // Activate alarm, must be done before PIN_StartProgram
    control.RegisterHandler(Handler, 0, FALSE);
    control.Activate();

    // Callback function "byeWorld" is invoked
    // right before Pin releases control of the application
    // to allow it to return to normal execution
    PIN_AddDetachFunction(helloWorld, 0);
    PIN_AddDetachFunction(byeWorld, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

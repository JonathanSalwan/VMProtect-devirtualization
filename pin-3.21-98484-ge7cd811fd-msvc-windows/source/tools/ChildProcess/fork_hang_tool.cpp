/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This application is part of the test: "fork_hang.test" which checks that no deadlock has occurred when
 * a child process tries to acquire the VM lock in a post fork callback.
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <sys/syscall.h>
using std::cerr;
using std::endl;

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool calls a post fork callback in a child process. It comes to check"
            "that no deadlock has occured when a child tries to acquire the VM lock in a post fork callback.\n";

    cerr << endl;

    return -1;
}

/* ===================================================================== */
/* Instrumentation function                                              */
/* ===================================================================== */

void fork_func(THREADID threadid, const CONTEXT* ctxt, VOID* v)
{
    /*
     * The function: "PIN_RemoveInstrumentation()" requires from the calling thread to hold the VM lock.
     */
    PIN_RemoveInstrumentation();
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    PIN_AddForkFunction(FPOINT_AFTER_IN_CHILD, fork_func, 0);

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

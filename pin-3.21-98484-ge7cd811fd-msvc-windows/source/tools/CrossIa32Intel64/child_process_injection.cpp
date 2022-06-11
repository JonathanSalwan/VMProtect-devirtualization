/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This tool is being used by the tests: "child_process_injection.test" and "child_process_injection1.test" .

#include "pin.H"
#include <stdio.h>
#include <iostream>
using std::cout;
using std::endl;

/* ===================================================================== */
VOID Fini(INT32 code, VOID* v) { cout << "End of tool" << endl; }

BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData)
{
    return TRUE; // run childProcess under Pin instrumentation
}

/* ===================================================================== */

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

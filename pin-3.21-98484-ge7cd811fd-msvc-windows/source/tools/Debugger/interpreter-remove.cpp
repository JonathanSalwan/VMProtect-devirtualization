/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Check the PIN_RemoveDebugInterpreter interface.
 */

#include <iostream>
#include "pin.H"
using std::cout;
using std::endl;

BOOL BadDebug(THREADID threadIndex, CONTEXT* ctxt, const std::string& cmd, std::string* reply, VOID* v)
{
    cout << "PIN_RemoveDebugInterpreter failed" << endl;
    return TRUE;
}

BOOL GoodDebug(THREADID threadIndex, CONTEXT* ctxt, const std::string& cmd, std::string* reply, VOID* v) { return TRUE; }

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    PIN_AddDebugInterpreter(BadDebug, 0);
    PIN_RemoveDebugInterpreter(BadDebug);
    PIN_AddDebugInterpreter(GoodDebug, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

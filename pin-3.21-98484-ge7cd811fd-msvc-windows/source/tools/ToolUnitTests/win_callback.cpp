/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This test tool verifies that Pin correctly handles Windows callbacks.
 * It also tests Pin_CallApplicationFunction API that is used within Windows callbacks.
 * The tool must be run with the "win_callback_app" application.
 */

#include "pin.H"
#include <string>
#include <iostream>

using std::cout;
using std::endl;
using std::hex;

/*!
 * RTN replacement routine.
 */
static VOID MySyscallInCallback(CONTEXT* ctxt, AFUNPTR pf)
{
    cout << "[win_callback] Calling SyscallInCallback() at " << hex << (VOID*)pf << endl;

    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, pf, NULL, PIN_PARG_END());

    cout << "Returned from SyscallInCallback()" << endl;
}

/*!
 * RTN instrumentation routine.
 */
static VOID InstrumentRoutine(RTN rtn, VOID*)
{
    if (RTN_Name(rtn) == "SyscallInCallback")
    {
        cout << "[win_callback] Replacing " << RTN_Name(rtn) << endl;

        PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "SyscallInCallback", PIN_PARG_END());

        RTN_ReplaceSignature(rtn, AFUNPTR(MySyscallInCallback), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_END);

        PROTO_Free(proto);
    }
}

/*
 * CONTEXT_CHANGE notification
 */
static void OnInterrupt(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT* ctxtFrom, CONTEXT* ctxtTo, INT32 info,
                        VOID* v)
{
    if (reason == CONTEXT_CHANGE_REASON_CALLBACK)
    {
        cout << "[win_callback] CONTEXT_CHANGE_REASON_CALLBACK" << endl;
    }
}

/*!
 * The main procedure of the tool.
 */
int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    RTN_AddInstrumentFunction(InstrumentRoutine, 0);
    PIN_AddContextChangeFunction(OnInterrupt, 0);

    PIN_StartProgram();
    return 0;
}

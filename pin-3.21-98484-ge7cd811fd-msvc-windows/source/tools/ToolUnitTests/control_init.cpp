/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that the CONTROL call-back for a thread happens after the thread-start call-back
 * for that thread.
 */

#include <stdlib.h>
#include "pin.H"
#include "control_manager.H"

using namespace CONTROLLER;

static VOID OnNewThread(THREADID, CONTEXT*, INT32, VOID*);
static VOID Handler(EVENT_TYPE, VOID*, CONTEXT*, VOID*, THREADID, bool);

CONTROL_MANAGER Control;

int main(int argc, char** argv)
{
    if (PIN_Init(argc, argv)) return 1;

    PIN_AddThreadStartFunction(OnNewThread, 0);
    Control.RegisterHandler(Handler, 0, FALSE);
    Control.Activate();

    PIN_StartProgram();
}

static BOOL SeenThreads[PIN_MAX_THREADS];

static VOID OnNewThread(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (tid < PIN_MAX_THREADS) SeenThreads[tid] = TRUE;
}

static VOID Handler(EVENT_TYPE ev, VOID* val, CONTEXT* ctxt, VOID* ip, THREADID tid, bool bcast)
{
    if (tid < PIN_MAX_THREADS && !SeenThreads[tid])
    {
        exit(1);
    }
}

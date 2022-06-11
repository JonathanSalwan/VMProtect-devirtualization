/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>

namespace WIND
{
#include <windows.h>
}

using std::cerr;
using std::endl;

/* ===================================================================== */
/* Global variables and declarations */
/* ===================================================================== */

static volatile int isAppStarted = 0;

static volatile int isReadyForDetach = 0;

static volatile int isReadyForAttach = 0;

static volatile int pinCompletelyAttached = 0;

/* ===================================================================== */

int rep_PinIsAttached() { return pinCompletelyAttached; }

int rep_PinIsDetached() { return 0; }

void rep_FirstProbeInvoked()
{
    cerr << "rep_FirstProbeInvoked" << endl;
    isReadyForDetach = 1;
}

void rep_SecondProbeInvoked()
{
    cerr << "rep_SecondProbeInvoked" << endl;
    isReadyForDetach = 1;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (isReadyForAttach == 1)
    {
        //can't get callbacks from pin after detach completion
        std::cerr << "failure - got follow child notification when pin is detached" << endl;
        exit(-1);
    }

    if (!IMG_IsMainExecutable(img))
    {
        return;
    }

    RTN rtn = RTN_FindByName(img, "PinIsAttached");
    ASSERTX(RTN_Valid(rtn));
    RTN_ReplaceProbed(rtn, AFUNPTR(rep_PinIsAttached));

    rtn = RTN_FindByName(img, "PinIsDetached");
    ASSERTX(RTN_Valid(rtn));
    RTN_ReplaceProbed(rtn, AFUNPTR(rep_PinIsDetached));

    rtn = RTN_FindByName(img, "FirstProbeInvoked");
    ASSERTX(RTN_Valid(rtn));
    RTN_ReplaceProbed(rtn, AFUNPTR(rep_FirstProbeInvoked));

    rtn = RTN_FindByName(img, "SecondProbeInvoked");
    ASSERTX(RTN_Valid(rtn));
    RTN_ReplaceProbed(rtn, AFUNPTR(rep_SecondProbeInvoked));

    pinCompletelyAttached = 1;
}

VOID DetachComplete(VOID* v)
{
    isReadyForDetach      = 0;
    isAppStarted          = 0;
    isReadyForAttach      = 1;
    pinCompletelyAttached = 0;
}

VOID AppStart(VOID* v) { isAppStarted = 1; }

VOID AttachMain(VOID* v)
{
    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_AddApplicationStartFunction(AppStart, 0);

    PIN_AddDetachFunctionProbed(DetachComplete, 0);

    isReadyForAttach = 0;
}

WIND::DWORD WINAPI ThreadProc(VOID* p)
{
    while (isReadyForDetach == 0)
    {
        WIND::SwitchToThread();
    }
    PIN_DetachProbed();

    while (isReadyForAttach == 0)
    {
        WIND::SwitchToThread();
    }

    PIN_AttachProbed(AttachMain, 0);

    while (isReadyForDetach == 0)
    {
        WIND::SwitchToThread();
    }
    PIN_DetachProbed();

    while (isReadyForAttach == 0)
    {
        WIND::SwitchToThread();
    }

    PIN_AttachProbed(AttachMain, 0);

    return 0;
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_AddApplicationStartFunction(AppStart, 0);

    PIN_AddDetachFunctionProbed(DetachComplete, 0);

    WIND::HANDLE threadHandle = WIND::CreateThread(NULL, 0, (WIND::LPTHREAD_START_ROUTINE)ThreadProc, NULL, 0, NULL);
    WIND::CloseHandle(threadHandle);

    // Never returns
    PIN_StartProgramProbed();

    return 0;
}

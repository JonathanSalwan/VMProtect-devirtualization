/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>

namespace WIND
{
#include <windows.h>
}
using std::cerr;
using std::cout;
using std::endl;
using std::string;
/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< BOOL > KnobReplaceOnReplace(KNOB_MODE_WRITEONCE, "pintool", "ror", "0", "check replace on replace");

KNOB< INT32 > KnobAttachCycles(KNOB_MODE_ACCUMULATE, "pintool", "attach_cycles", "0", "number of detach & re-attach cycles");

KNOB< INT32 > KnobFollowChildEvents(KNOB_MODE_ACCUMULATE, "pintool", "fc", "0",
                                    "number of follow child events to complete attach cycle");

/* ===================================================================== */
/* Global variables and declarations */
/* ===================================================================== */

typedef void(__cdecl* SHORT_FUNCTION_TYPE)(size_t size);

typedef int(__cdecl* DO_LOOP_TYPE)();

static volatile int doLoopPred = 1;

static volatile int globalCounter1 = 0;

static volatile int globalCounter2 = 0;

static volatile int isAppStarted = 0;

static volatile int isReadyForDetach = 0;

static volatile int isReadyForAttach = 0;

static volatile int followChildCounter = 0;

static volatile int remainingAttachCycles = 0;

/* ===================================================================== */
int rep_DoLoop()
{
    PIN_LockClient();

    volatile int localPred = doLoopPred;

    PIN_UnlockClient();

    return localPred;
}

/* ===================================================================== */

VOID rep_ShortFunction1(SHORT_FUNCTION_TYPE pShortFunction1, WIND::DWORD h)
{
    WIND::DWORD h_ref = KnobReplaceOnReplace ? 2 : 1;
    if (h != h_ref)
    {
        std::cerr << "failure - wrong h value = " << h << endl;
        exit(-1);
    }

    pShortFunction1(1);

    PIN_LockClient();

    globalCounter1++;

    if (globalCounter1 >= 100 && globalCounter2 >= 100 && isAppStarted == 1 && doLoopPred != 0 &&
        followChildCounter >= KnobFollowChildEvents.Value())
    {
        if (remainingAttachCycles == 0)
        {
            //eventhough this is not an error - print to cerr (in order to see it on the screen)
            std::cerr << "success - exiting from application!" << endl;
            doLoopPred = 0;
        }
        else if (isReadyForDetach == 0)
        {
            isReadyForDetach = 1;
        }
    }

    PIN_UnlockClient();
}

VOID rep_rep_ShortFunction1(SHORT_FUNCTION_TYPE pRep_ShortFunction1, WIND::DWORD h)
{
    if (!KnobReplaceOnReplace)
    {
        std::cerr << "failure - should never get here if replace on replace is disbaled " << endl;
    }
    //Verify original value
    if (h != 1)
    {
        std::cerr << "failure - wrong h value = " << h << endl;
        exit(-1);
    }
    pRep_ShortFunction1(h * 2);
}

/* ===================================================================== */
SHORT_FUNCTION_TYPE pShortFunction2;

VOID rep_ShortFunction2(WIND::DWORD h)
{
    WIND::DWORD h_ref = KnobReplaceOnReplace ? 4 : 2;
    if (h != h_ref)
    {
        std::cerr << "failure - wrong h value = " << h << endl;
        exit(-1);
    }
    pShortFunction2(2);

    PIN_LockClient();

    globalCounter2++;

    if (globalCounter1 >= 100 && globalCounter2 >= 100 && isAppStarted == 1 && doLoopPred != 0 &&
        followChildCounter >= KnobFollowChildEvents.Value())
    {
        if (remainingAttachCycles == 0)
        {
            //eventhough this is not an error - print to cerr (in order to see it on the screen)
            std::cerr << "success - exiting from application!" << endl;
            doLoopPred = 0;
        }
        else if (isReadyForDetach == 0)
        {
            isReadyForDetach = 1;
        }
    }

    PIN_UnlockClient();
}

/* ===================================================================== */
SHORT_FUNCTION_TYPE pShortFunction2Rep;

VOID rep_rep_ShortFunction2(WIND::DWORD h)
{
    if (!KnobReplaceOnReplace)
    {
        std::cerr << "failure - should never get here if replace on replace is disbaled " << endl;
    }
    //Verify original value
    if (h != 2)
    {
        std::cerr << "failure - wrong h value = " << h << endl;
        exit(-1);
    }
    pShortFunction2Rep(h * 2);
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

    cout << IMG_Name(img) << endl;

    if (!IMG_IsMainExecutable(img))
    {
        return;
    }

    PROTO proto_ShortFunction =
        PROTO_Allocate(PIN_PARG(void), CALLINGSTD_CDECL, "ShortFunction1", PIN_PARG(WIND::DWORD), PIN_PARG_END());

    const string sFuncName1("ShortFunction1");
    const string sFuncName2("ShortFunction2");
    const string sFuncName3("DoLoop");

    for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym))
    {
        string undFuncName = PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY);
        if (undFuncName == sFuncName1)
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn) && RTN_IsSafeForProbedReplacement(rtn))
            {
                //eventhough this is not an error - print to cerr (in order to see it on the screen)
                cerr << "Inserting first probe in ShortFunction1() in " << IMG_Name(img) << endl;

                RTN_ReplaceSignatureProbed(rtn, AFUNPTR(rep_ShortFunction1), IARG_PROTOTYPE, proto_ShortFunction,
                                           IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            }
            if (KnobReplaceOnReplace && RTN_Valid(rtn) && RTN_IsSafeForProbedReplacement(rtn))
            {
                //eventhough this is not an error - print to cerr (in order to see it on the screen)
                cerr << "Inserting second probe in ShortFunction1() in " << IMG_Name(img) << endl;

                RTN_ReplaceSignatureProbed(rtn, AFUNPTR(rep_rep_ShortFunction1), IARG_PROTOTYPE, proto_ShortFunction,
                                           IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            }
        }
        if (undFuncName == sFuncName2)
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn) && RTN_IsSafeForProbedReplacement(rtn))
            {
                //eventhough this is not an error - print to cerr (in order to see it on the screen)
                cerr << "Inserting first probe in ShortFunction2() in " << IMG_Name(img) << endl;

                pShortFunction2 = (SHORT_FUNCTION_TYPE)RTN_ReplaceProbed(rtn, AFUNPTR(rep_ShortFunction2));
            }
            if (KnobReplaceOnReplace && RTN_Valid(rtn) && RTN_IsSafeForProbedReplacement(rtn))
            {
                //eventhough this is not an error - print to cerr (in order to see it on the screen)
                cerr << "Inserting second probe in ShortFunction2() in " << IMG_Name(img) << endl;

                pShortFunction2Rep = (SHORT_FUNCTION_TYPE)RTN_ReplaceProbed(rtn, AFUNPTR(rep_rep_ShortFunction2));
            }
        }
        if (undFuncName == sFuncName3)
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn) && RTN_IsSafeForProbedReplacement(rtn))
            {
                //eventhough this is not an error - print to cerr (in order to see it on the screen)
                cerr << "Inserting a probe in DoLoop() in " << IMG_Name(img) << endl;

                RTN_ReplaceProbed(rtn, AFUNPTR(rep_DoLoop));
            }
        }
    }

    PROTO_Free(proto_ShortFunction);
}

BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData)
{
    if (isReadyForAttach == 1)
    {
        //can't get callbacks from pin after detach completion
        std::cerr << "failure - got follow child notification when pin is detached" << endl;
        exit(-1);
    }
    followChildCounter++;
    return TRUE;
}

VOID DetachComplete(VOID* v)
{
    std::cerr << "Detach complete" << endl;
    isReadyForDetach   = 0;
    isAppStarted       = 0;
    followChildCounter = 0;
    isReadyForAttach   = 1;
}

VOID AppStart(VOID* v)
{
    std::cerr << "Application started" << endl;

    isAppStarted = 1;
}

VOID AttachMain(VOID* v)
{
    std::cerr << "Attach main, Number of remaining attach cycles = " << remainingAttachCycles << endl;

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_AddApplicationStartFunction(AppStart, 0);

    PIN_AddDetachFunctionProbed(DetachComplete, 0);

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    globalCounter1   = 0;
    globalCounter2   = 0;
    isReadyForAttach = 0;
}

WIND::DWORD WINAPI ThreadProc(VOID* p)
{
    while (remainingAttachCycles > 0)
    {
        while (isReadyForDetach == 0)
        {
            WIND::Sleep(10);
        }

        PIN_DetachProbed();

        while (isReadyForAttach == 0)
        {
            WIND::Sleep(10);
        }

        remainingAttachCycles--;

        PIN_AttachProbed(AttachMain, 0);
    }

    return 0;
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_AddApplicationStartFunction(AppStart, 0);

    PIN_AddDetachFunctionProbed(DetachComplete, 0);

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    remainingAttachCycles = KnobAttachCycles.Value();
    if (remainingAttachCycles > 0)
    {
        WIND::HANDLE threadHandle = WIND::CreateThread(NULL, 0, (WIND::LPTHREAD_START_ROUTINE)ThreadProc, NULL, 0, NULL);
        WIND::CloseHandle(threadHandle);
        threadHandle = NULL;
    }

    // Never returns
    PIN_StartProgramProbed();

    return 0;
}

/*
 * Copyright (C) 2009-2021 Intel Corporation.
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
using std::flush;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

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

static volatile int followChildCounter = 0;

static volatile int isAppStarted = 0;

/* ===================================================================== */

int rep_DoLoop()
{
    PIN_LockClient();

    volatile int localPred = doLoopPred;

    PIN_UnlockClient();

    return localPred;
}

/* ===================================================================== */

VOID before_ShortFunction1()
{
    PIN_LockClient();

    globalCounter1++;

    if (isAppStarted && globalCounter1 >= 100 && globalCounter2 >= 100 && doLoopPred != 0 &&
        followChildCounter >= KnobFollowChildEvents.Value())
    {
        //eventhough this is not an error - print to cerr (in order to see it on the screen)
        std::cerr << "success - exiting from application!" << endl << flush;
        doLoopPred = 0;
    }

    PIN_UnlockClient();
}

/* ===================================================================== */

VOID before_ShortFunction2()
{
    PIN_LockClient();

    globalCounter2++;

    if (isAppStarted && globalCounter1 >= 100 && globalCounter2 >= 100 && doLoopPred != 0 &&
        followChildCounter >= KnobFollowChildEvents.Value())
    {
        //eventhough this is not an error - print to cerr (in order to see it on the screen)
        std::cerr << "success - exiting from application!" << endl << flush;
        doLoopPred = 0;
    }

    PIN_UnlockClient();
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img))
    {
        return;
    }

    cout << endl << IMG_Name(img) << endl << flush;

    const string sFuncName1("ShortFunction1");
    const string sFuncName2("ShortFunction2");
    const string sFuncName3("DoLoop");

    for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym))
    {
        string undFuncName = PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY);
        if (undFuncName == sFuncName1)
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn))
            {
                //eventhough this is not an error - print to cerr (in order to see it on the screen)
                cerr << "Inserting analysis function before ShortFunction1() in " << IMG_Name(img) << endl << flush;

                RTN_Open(rtn);

                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(before_ShortFunction1), IARG_END);

                RTN_Close(rtn);
            }
        }
        if (undFuncName == sFuncName2)
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn))
            {
                //eventhough this is not an error - print to cerr (in order to see it on the screen)
                cerr << "Inserting analysis function before ShortFunction2() in " << IMG_Name(img) << endl << flush;

                RTN_Open(rtn);

                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(before_ShortFunction2), IARG_END);

                RTN_Close(rtn);
            }
        }
        if (undFuncName == sFuncName3)
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn))
            {
                //eventhough this is not an error - print to cerr (in order to see it on the screen)
                cerr << "Replacing DoLoop() in " << IMG_Name(img) << endl << flush;

                RTN_Replace(rtn, AFUNPTR(rep_DoLoop));
            }
        }
    }
}

BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData)
{
    followChildCounter++;
    return TRUE;
}

VOID AppStart(VOID* v)
{
    std::cerr << "Application started" << endl << flush;

    isAppStarted = 1;
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    PIN_AddApplicationStartFunction(AppStart, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

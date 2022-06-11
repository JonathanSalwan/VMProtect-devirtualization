/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This test tool instruments every instruction of the sw_interrupt_app program
 * to check correctness of the context recovery for software interrupts.
 */

#include "pin.H"
#include <string>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
/* ===================================================================== */
/* Global variables and declarations */
/* ===================================================================== */
typedef int(__cdecl* DO_LOOP_TYPE)();

static volatile int doLoopPred = 1;

/* ===================================================================== */

int rep_DoLoop()
{
    PIN_LockClient();

    volatile int localPred = doLoopPred;

    PIN_UnlockClient();

    return localPred;
}

/*!
 * Context change callback.
 */
static void OnContextChange(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT* ctxtFrom, CONTEXT* ctxtTo,
                            INT32 info, VOID* v)
{
    PIN_LockClient();
    static volatile INT32 contextChnageCount = 0;
    contextChnageCount++;
    if (reason == CONTEXT_CHANGE_REASON_EXCEPTION)
    {
#if 0
        UINT32 exceptionCode = info;
        ADDRINT exceptAddr = PIN_GetContextReg(ctxtFrom, REG_INST_PTR);
        cerr << "CONTEXT_CHANGE_REASON_EXCEPTION: " << 
            "Exception code " << hex << exceptionCode << "." <<
            "Context IP " << hex << exceptAddr << "." <<
            endl;
#endif
    }
    if (contextChnageCount == 20)
    {
        doLoopPred = 0;
    }
    PIN_UnlockClient();
}

VOID ImageLoad(IMG img, VOID* v)
{
    cout << IMG_Name(img) << endl;

    if (!IMG_IsMainExecutable(img))
    {
        return;
    }
    const string sFuncName("DoLoop");

    for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym))
    {
        string undFuncName = PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY);
        if (undFuncName == sFuncName)
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn))
            {
                //eventhough this is not an error - print to cerr (in order to see it on the screen)
                cerr << "Replacing DoLoop() in " << IMG_Name(img) << endl;

                RTN_Replace(rtn, AFUNPTR(rep_DoLoop));
            }
        }
    }
}

/*!
 * The main procedure of the tool.
 */
int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddContextChangeFunction(OnContextChange, 0);

    PIN_StartProgram();
    return 0;
}

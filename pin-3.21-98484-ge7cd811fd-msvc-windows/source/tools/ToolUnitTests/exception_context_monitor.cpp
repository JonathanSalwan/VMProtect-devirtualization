/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  A pin tool that intercepts exception and verifies the exception context.
 *  It works in pair with the win_exception_context application, that raises two 
 *  exceptions. The context of the second exception should have a predefined FP 
 *  state: all FPn and XMMn registers have value <n> in their first byte. 
 *  This is verified by the tool.
 */

#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <memory.h>
#include "pin.H"

using std::cerr;
using std::endl;
using std::ofstream;
using std::string;
/*
 * Verify the FP state of the exception context
 */
KNOB< BOOL > KnobCheckFp(KNOB_MODE_WRITEONCE, "pintool", "checkfp", "0", "Check FP state");
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "exception_context_monitor_tool.out", "specify file name");

static ofstream* logFile = NULL;

ADDRINT appMainStart = 0;
ADDRINT appMainEnd   = 0;

/*!
 * Exit with the specified error message
 */
static void Abort(const char* msg)
{
    cerr << "Tool: " << msg << endl;
    PIN_ExitProcess(1);
}

/*!
 * Check to see that FP/XMM registers in the specified context have predefined
 * values assigned by the application: FPn and XMMn registers have value <n> in 
 * their first byte. 
 */
static bool CheckMyFpContext(const CONTEXT* pContext)
{
    unsigned char fpContextSpaceForXsave[sizeof(FPSTATE) + FPSTATE_ALIGNMENT];
    FPSTATE* fpState = reinterpret_cast< FPSTATE* >(
        (reinterpret_cast< ADDRINT >(fpContextSpaceForXsave) + (FPSTATE_ALIGNMENT - 1)) & (-1 * FPSTATE_ALIGNMENT));

    PIN_GetContextFPState(pContext, fpState);

    for (unsigned i = 0; i < 8; ++i)
    {
        if (fpState->fxsave_legacy._sts[i]._raw._lo != i) return false;
        if (fpState->fxsave_legacy._sts[i]._raw._hi != 0) return false;
    }

    unsigned numXmms = sizeof(fpState->fxsave_legacy._xmms) / sizeof(fpState->fxsave_legacy._xmms[0]);
    for (unsigned i = 0; i < numXmms; ++i)
    {
        if (fpState->fxsave_legacy._xmms[i]._vec64[0] != i) return false;
        if (fpState->fxsave_legacy._xmms[i]._vec64[1] != 0) return false;
    }

    return true;
}

static int hadException = FALSE;
static void OnException(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT* ctxtFrom, CONTEXT* ctxtTo, INT32 info,
                        VOID* v)
{
    // check if this is an Exception from our app code
    ADDRINT exceptionAddr = PIN_GetContextReg(ctxtFrom, REG_INST_PTR);
    *logFile << "got exception at address " << hexstr(exceptionAddr) << " reason: " << hexstr(reason) << endl;
    if (reason == CONTEXT_CHANGE_REASON_EXCEPTION)
    {
        if (exceptionAddr < appMainStart || exceptionAddr > appMainEnd) return;

        *logFile << "handling exception" << endl;
        hadException      = TRUE;
        static bool first = true;

        if (first)
        {
            *logFile << "first time" << endl;
            first = false;
        }
        else
        {
            if (KnobCheckFp)
            {
                *logFile << "tool: checking FP" << endl;
                if (!CheckMyFpContext(ctxtFrom)) Abort("Mismatch in the FP context");
            }
        }
    }
}

// Image load callback for the first Pin session
VOID ImageLoad(IMG img, VOID* v)
{
    *logFile << "img load " << IMG_Name(img) << endl;
    if (IMG_IsMainExecutable(img))
    {
        RTN mainRtn = RTN_FindByName(img, "main");
        if (!RTN_Valid(mainRtn)) Abort("could not find main in application");

        appMainStart = RTN_Address(mainRtn);
        appMainEnd   = RTN_Size(mainRtn) + appMainStart;
        *logFile << "main exec range " << hexstr(appMainStart) << "-" << hexstr(appMainEnd) << endl;
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (!hadException) Abort("Did not detect exception");
}

int main(INT32 argc, CHAR** argv)
{
    if (PIN_Init(argc, argv)) Abort("Invalid arguments");

    PIN_InitSymbols();

    logFile = new std::ofstream(KnobOutputFile.Value().c_str());

    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddContextChangeFunction(OnException, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

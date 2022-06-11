/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
  Replace an original function with a custom function defined in the tool. 
  Call the original function from the replacement function using
  PIN_CallApplicationFunction
  Verify that the original function and functions it calls are instrumented

*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>
#include <stdlib.h>
#include <fstream>
using std::cerr;
using std::endl;
using std::ofstream;
using std::string;

static KNOB< string > KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "callapp14.out", "output file");

ADDRINT functionCalledByFunctionToBeReplacedAddr               = 0;
ADDRINT functionToBeReplacedAddr                               = 0;
BOOL replacementFunctionCalled                                 = FALSE;
BOOL replacementDone                                           = FALSE;
BOOL functionToBeReplacedInstrumented                          = FALSE;
BOOL functionToBeReplacedInstrumentationCalled                 = FALSE;
BOOL functionCalledByFunctionToBeReplacedInstrumented          = FALSE;
BOOL functionCalledByFunctionToBeReplacedInstrumentationCalled = FALSE;
static ofstream out;

/* ===================================================================== */

int MyReplacementFunction(CONTEXT* ctxt, AFUNPTR origPtr, int one, int two)
{
    out << " MyReplacementFunction: PIN_CallApplicationFunction Replaced Function at address " << hexstr(ADDRINT(origPtr))
        << endl;

    int res;

    replacementFunctionCalled = TRUE;
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, origPtr, NULL, PIN_PARG(int), &res, PIN_PARG(int), one,
                                PIN_PARG(int), two, PIN_PARG_END());

    out << " MyReplacementFunction: Returned from Replaced Function res = " << res << endl;

    return res;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    PROTO proto =
        PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "FunctionToBeReplaced", PIN_PARG(int), PIN_PARG(int), PIN_PARG_END());

    RTN rtn = RTN_FindByName(img, "FunctionToBeReplaced");
    if (RTN_Valid(rtn))
    {
        out << " RTN_ReplaceSignature " << RTN_Name(rtn) << " in " << IMG_Name(img) << " at address " << hexstr(RTN_Address(rtn))
            << " with MyReplacementFunction" << endl;

        functionToBeReplacedAddr = RTN_Address(rtn);

        RTN_ReplaceSignature(rtn, AFUNPTR(MyReplacementFunction), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_UINT32, 1, IARG_UINT32, 2, IARG_END);

        RTN rtn2 = RTN_FindByName(img, "FunctionCalledByFunctionToBeReplaced");
        if (RTN_Valid(rtn2))
        {
            functionCalledByFunctionToBeReplacedAddr = RTN_Address(rtn2);
        }

        replacementDone = TRUE;
    }
    PROTO_Free(proto);
}

VOID FunctionToBeReplacedAnalysisFunc() { functionToBeReplacedInstrumentationCalled = TRUE; }

VOID FunctionCalledByFunctionToBeReplacedAnalysisFunc() { functionCalledByFunctionToBeReplacedInstrumentationCalled = TRUE; }

VOID Instruction(INS ins, VOID* v)
{
    if (INS_Address(ins) == functionToBeReplacedAddr)
    {
        functionToBeReplacedInstrumented = TRUE;
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)FunctionToBeReplacedAnalysisFunc, IARG_END);
    }
    else if (INS_Address(ins) == functionCalledByFunctionToBeReplacedAddr)
    {
        functionCalledByFunctionToBeReplacedInstrumented = TRUE;
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)FunctionCalledByFunctionToBeReplacedAnalysisFunc, IARG_END);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    BOOL hadError = FALSE;
    if (!replacementDone)
    {
        out << "***Error !replacementDone" << endl;
        hadError = TRUE;
    }
    if (!functionToBeReplacedInstrumented)
    {
        out << "***Error !functionToBeReplacedInstrumented" << endl;
        hadError = TRUE;
    }
    if (!functionCalledByFunctionToBeReplacedInstrumented)
    {
        out << "***Error !functionCalledByFunctionToBeReplacedInstrumented" << endl;
        hadError = TRUE;
    }
    if (!functionToBeReplacedInstrumentationCalled)
    {
        out << "***Error !functionToBeReplacedInstrumentationCalled" << endl;
        hadError = TRUE;
    }
    if (!functionCalledByFunctionToBeReplacedInstrumentationCalled)
    {
        out << "***Error !functionCalledByFunctionToBeReplacedInstrumentationCalled" << endl;
        hadError = TRUE;
    }
    if (hadError)
    {
        out << "***Error hadError" << endl;
        exit(-1);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "Tool: callapp14" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
int main(INT32 argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv)) return Usage();

    out.open(KnobOutput.Value().c_str());

    IMG_AddInstrumentFunction(ImageLoad, 0);

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

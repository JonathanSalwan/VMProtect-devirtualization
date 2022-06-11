/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
 * Insert a call before and after a stdcall function in probe mode.
 */

/* ===================================================================== */
#include "pin.H"

#if defined(TARGET_WINDOWS)
namespace WINDOWS
{
#include <Windows.h>
}
#endif
#include <stdio.h>
#include "probe_stdcall_fastcall.h"

using std::string;
/* ===================================================================== */
typedef int(__stdcall* FuncType)(int, int, int, int, int, int, int, int, int, int);

class EXECUTION_CHECKER
{
  public:
    EXECUTION_CHECKER()
    {
        _instrumentedFunc1             = FALSE;
        _instrumentedFunc2             = FALSE;
        _instrumentedFunc3             = FALSE;
        _instrumentedFunc4             = FALSE;
        _instrumentedFunc5             = FALSE;
        _numBeforeExecutions           = 0;
        _numBeforeWithParamsExecutions = 0;
        _numAfterExecutions            = 0;
        _numAfterWithParamsExecutions  = 0;
    }

    ~EXECUTION_CHECKER()
    {
        if (!_instrumentedFunc1)
        {
            printf("did not instrument func1\n");
            exit(1);
        }
        if (!_instrumentedFunc2)
        {
            printf("did not instrument func2\n");
            exit(1);
        }
        if (!_instrumentedFunc3)
        {
            printf("did not instrument func3\n");
            exit(1);
        }
        if (!_instrumentedFunc4)
        {
            printf("did not instrument func4\n");
            exit(1);
        }
        if (!_instrumentedFunc5)
        {
            printf("did not instrument func5\n");
            exit(1);
        }
        if (_numBeforeExecutions != 2)
        {
            printf("did not execute expected # of BeforeExecutions\n");
            exit(1);
        }
        if (_numBeforeWithParamsExecutions != 2)
        {
            printf("did not execute expected # of BeforeWithParamsExecutions\n");
            exit(1);
        }
        if (_numAfterWithParamsExecutions != 1)
        {
            printf("did not execute expected # of AfterWithParamsExecutions\n");
            exit(1);
        }
        if (_numAfterExecutions != 4)
        {
            printf("did not execute expected # of AfterExecutions\n");
            exit(1);
        }
    }
    BOOL _instrumentedFunc1;
    BOOL _instrumentedFunc2;
    BOOL _instrumentedFunc3;
    BOOL _instrumentedFunc4;
    BOOL _instrumentedFunc5;
    int _numBeforeExecutions;
    int _numBeforeWithParamsExecutions;
    int _numAfterExecutions;
    int _numAfterWithParamsExecutions;
};
EXECUTION_CHECKER executionChecker;
/* ===================================================================== */
/* Analysis routines  */
/* ===================================================================== */

VOID Before() { executionChecker._numBeforeExecutions++; }

VOID BeforeWithParams(char c1, int num1, char c2, int num2, char c3)
{
    executionChecker._numBeforeWithParamsExecutions++;
    if (c1 != CHAR_VAL1)
    {
        printf("TOOL ERROR: BeforeWithParams was not passed in c1 as expected, actual c1 %c\n", c1);
        exit(1);
    }
    if (c2 != CHAR_VAL2)
    {
        printf("TOOL ERROR: BeforeWithParams was not passed in c2  as expected, actual c2 %c\n", c2);
        exit(1);
    }
    if (c3 != CHAR_VAL3)
    {
        printf("TOOL ERROR: BeforeWithParams was not passed in c3  as expected, actual c3 %c %x\n", c3, c3);
        exit(1);
    }
    if (num1 != INT_VAL1)
    {
        printf("TOOL ERROR: BeforeWithParams was not passed in num1 as expected, actual num1 %d\n", num1);
        exit(1);
    }
    if (num2 != INT_VAL2)
    {
        printf("TOOL ERROR: BeforeWithParams was not passed in num2 as expected, actual num2 %d\n", num2);
        exit(1);
    }
}

VOID After() { executionChecker._numAfterExecutions++; }

VOID AfterWithParams(int retValFromStdCallFunction5ToBeReplacedByPinAppFun)
{
    executionChecker._numAfterWithParamsExecutions++;
    if (retValFromStdCallFunction5ToBeReplacedByPinAppFun != RETVAL5)
    {
        printf("TOOL ERROR: AfterWithParams was not passed in retValFromStdCallFunction5ToBeReplacedByPinAppFun as expected, "
               "actual retValFromStdCallFunction5ToBeReplacedByPinAppFun %d\n",
               retValFromStdCallFunction5ToBeReplacedByPinAppFun);
        exit(1);
    }
}

/* ===================================================================== */
/* Instrumentation routines  */
/* ===================================================================== */

VOID Sanity(IMG img, RTN rtn)
{
    if (PIN_IsProbeMode() && !RTN_IsSafeForProbedInsertion(rtn))
    {
        printf("Cannot insert calls around %s() in %s\n", RTN_Name(rtn).c_str(), IMG_Name(img).c_str());
        exit(1);
    }
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    // Walk through the symbols in the symbol table.
    //
    for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym))
    {
        string undFuncName = PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY);

        //  Find the LocalAlloc function.
        if (undFuncName == "StdCallFunctionToBeReplacedByPin")
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn))
            {
                Sanity(img, rtn);

                printf("Inserting calls before/after StdCallFunctionToBeReplacedByPin() in %s at address %x\n",
                       IMG_Name(img).c_str(), RTN_Address(rtn));

                PROTO protoOfStdCallFunction1ToBeReplacedByPin =
                    PROTO_Allocate(PIN_PARG(int), CALLINGSTD_STDCALL, "protoOfStdCallFunction1ToBeReplacedByPin", PIN_PARG(char),
                                   PIN_PARG(int), PIN_PARG(char), PIN_PARG(int), PIN_PARG(char), PIN_PARG_END());

                RTN_InsertCallProbed(rtn, IPOINT_AFTER, AFUNPTR(After), IARG_PROTOTYPE, protoOfStdCallFunction1ToBeReplacedByPin,
                                     IARG_END);

                RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before), IARG_PROTOTYPE,
                                     protoOfStdCallFunction1ToBeReplacedByPin, IARG_END);

                PROTO_Free(protoOfStdCallFunction1ToBeReplacedByPin);
                executionChecker._instrumentedFunc1 = TRUE;
            }
        }

        else if (undFuncName == "StdCallFunction2ToBeReplacedByPin")
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn))
            {
                Sanity(img, rtn);

                printf("Inserting call before StdCallFunction2ToBeReplacedByPin() in %s at address %x\n", IMG_Name(img).c_str(),
                       RTN_Address(rtn));

                PROTO protoOfStdCallFunction2ToBeReplacedByPin =
                    PROTO_Allocate(PIN_PARG(int), CALLINGSTD_STDCALL, "protoOfStdCallFunction2ToBeReplacedByPin", PIN_PARG(char),
                                   PIN_PARG(int), PIN_PARG(char), PIN_PARG(int), PIN_PARG(char), PIN_PARG_END());

                RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before), IARG_PROTOTYPE,
                                     protoOfStdCallFunction2ToBeReplacedByPin, IARG_END);

                RTN_InsertCallProbed(rtn, IPOINT_AFTER, AFUNPTR(After), IARG_PROTOTYPE, protoOfStdCallFunction2ToBeReplacedByPin,
                                     IARG_END);

                PROTO_Free(protoOfStdCallFunction2ToBeReplacedByPin);
                executionChecker._instrumentedFunc2 = TRUE;
            }
        }

        else if (undFuncName == "StdCallFunction3ToBeReplacedByPin")
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn))
            {
                Sanity(img, rtn);

                printf("Inserting call after StdCallFunction3ToBeReplacedByPin() in %s at address %x\n", IMG_Name(img).c_str(),
                       RTN_Address(rtn));

                PROTO protoOfStdCallFunction3ToBeReplacedByPin =
                    PROTO_Allocate(PIN_PARG(int), CALLINGSTD_STDCALL, "protoOfStdCallFunction3ToBeReplacedByPin", PIN_PARG(char),
                                   PIN_PARG(int), PIN_PARG(char), PIN_PARG(int), PIN_PARG(char), PIN_PARG_END());

                RTN_InsertCallProbed(rtn, IPOINT_AFTER, AFUNPTR(After), IARG_PROTOTYPE, protoOfStdCallFunction3ToBeReplacedByPin,
                                     IARG_END);

                PROTO_Free(protoOfStdCallFunction3ToBeReplacedByPin);
                executionChecker._instrumentedFunc3 = TRUE;
            }
        }

        else if (undFuncName == "StdCallFunction4ToBeReplacedByPin")
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn))
            {
                Sanity(img, rtn);

                printf("Inserting call after StdCallFunction4ToBeReplacedByPin() in %s at address %x\n", IMG_Name(img).c_str(),
                       RTN_Address(rtn));

                PROTO protoOfStdCallFunction4ToBeReplacedByPin =
                    PROTO_Allocate(PIN_PARG(int), CALLINGSTD_STDCALL, "protoOfStdCallFunction4ToBeReplacedByPin", PIN_PARG(char),
                                   PIN_PARG(int), PIN_PARG(char), PIN_PARG(int), PIN_PARG(char), PIN_PARG_END());

                RTN_InsertCallProbed(rtn, IPOINT_AFTER, AFUNPTR(After), IARG_PROTOTYPE, protoOfStdCallFunction4ToBeReplacedByPin,
                                     IARG_END);

                RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(BeforeWithParams), IARG_PROTOTYPE,
                                     protoOfStdCallFunction4ToBeReplacedByPin, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                                     IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                                     IARG_FUNCARG_ENTRYPOINT_VALUE, 3, IARG_FUNCARG_ENTRYPOINT_VALUE, 4, IARG_END);

                PROTO_Free(protoOfStdCallFunction4ToBeReplacedByPin);
                executionChecker._instrumentedFunc4 = TRUE;
            }
        }
        else if (undFuncName == "StdCallFunction5ToBeReplacedByPin")
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn))
            {
                Sanity(img, rtn);

                printf("Inserting call after StdCallFunction5ToBeReplacedByPin() in %s at address %x\n", IMG_Name(img).c_str(),
                       RTN_Address(rtn));

                PROTO protoOfStdCallFunction5ToBeReplacedByPin =
                    PROTO_Allocate(PIN_PARG(int), CALLINGSTD_STDCALL, "protoOfStdCallFunction5ToBeReplacedByPin", PIN_PARG(char),
                                   PIN_PARG(int), PIN_PARG(char), PIN_PARG(int), PIN_PARG(char), PIN_PARG_END());

                RTN_InsertCallProbed(rtn, IPOINT_AFTER, AFUNPTR(AfterWithParams), IARG_PROTOTYPE,
                                     protoOfStdCallFunction5ToBeReplacedByPin, IARG_REG_VALUE, REG_GAX, IARG_END);

                RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(BeforeWithParams), IARG_PROTOTYPE,
                                     protoOfStdCallFunction5ToBeReplacedByPin, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                                     IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                                     IARG_FUNCARG_ENTRYPOINT_VALUE, 3, IARG_FUNCARG_ENTRYPOINT_VALUE, 4, IARG_END);

                PROTO_Free(protoOfStdCallFunction5ToBeReplacedByPin);
                executionChecker._instrumentedFunc5 = TRUE;
            }
        }
    }
}

/* ===================================================================== */

int main(INT32 argc, CHAR* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

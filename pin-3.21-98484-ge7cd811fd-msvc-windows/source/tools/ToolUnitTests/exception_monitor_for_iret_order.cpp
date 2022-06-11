/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
This test verifies order of analysis calls on the iret instruction -
iret is special because it is an emulated taken path instruction
*/
#include <stdio.h>
#include "pin.H"

using std::string;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "exception_monitor_for_iret_order.out", "Output file");

FILE* out;

void BeforePinVerifyInTry()
{
    fprintf(out, "BeforePinVerifyInTry\n");
    fflush(out);
}

void BeforePinVerifyInCatch()
{
    fprintf(out, "BeforePinVerifyInCatch\n");
    fflush(out);
}

void BeforePinVerifyAfterCatch()
{
    fprintf(out, "BeforePinVerifyAfterCatch\n");
    fflush(out);
}

void BeforePinVerifyInDestructor()
{
    fprintf(out, "BeforePinVerifyInDestructor\n");
    fflush(out);
}

BOOL iretBeforeCalled         = FALSE;
BOOL iretBeforeCalledAtIretIf = FALSE;
BOOL ifCalled                 = FALSE;
BOOL thenCalled               = FALSE;
BOOL iretQAfter1Called        = FALSE;
BOOL iretQAfter2Called        = FALSE;
BOOL instrumentedIretq        = FALSE;

VOID Fini(INT32 code, VOID* v)
{
    fprintf(out, "PinFiniFunction\n");
    if (!instrumentedIretq)
    {
        printf("***Error !instrumentedIretq\n");
        exit(-1);
    }
    if (!iretBeforeCalled)
    {
        printf("***Error !iretBeforeCalled\n");
        exit(-1);
    }
    if (!iretBeforeCalledAtIretIf)
    {
        printf("***Error !iretBeforeCalledAtIretIf\n");
        exit(-1);
    }
    if (!ifCalled)
    {
        printf("***Error !ifCalled\n");
        exit(-1);
    }
    if (!thenCalled)
    {
        printf("***Error !thenCalled\n");
        exit(-1);
    }
    if (!iretQAfter1Called)
    {
        printf("***Error !iretQAfter1Called\n");
        exit(-1);
    }
    if (!iretQAfter2Called)
    {
        printf("***Error !iretQAfter2Called\n");
        exit(-1);
    }
    fclose(out);
}

static void OnException(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT* ctxtFrom, CONTEXT* ctxtTo, INT32 info,
                        VOID* v)
{
    if (reason == CONTEXT_CHANGE_REASON_EXCEPTION)
    {
        UINT32 exceptionCode = info;
        // Depending on the system and CRT version, C++ exceptions can be implemented
        // as kernel- or user-mode- exceptions.
        // This callback does not not intercept user mode exceptions, so we do not
        // log C++ exceptions to avoid difference in output files.
        if ((exceptionCode >= 0xc0000000) && (exceptionCode <= 0xcfffffff))
        {
            fprintf(out, "Start handling exception. Exception code = 0x%x\n", exceptionCode);
            fflush(out);
        }
    }
}

VOID Image(IMG img, VOID* v)
{
    return;
    // hook the functions in the image. If these functions are called then it means
    // that pin has not lost control.
    RTN rtn = RTN_FindByName(img, "PinVerifyInTry");
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforePinVerifyInTry), IARG_END);
        RTN_Close(rtn);
    }
    rtn = RTN_FindByName(img, "PinVerifyInCatch");
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforePinVerifyInCatch), IARG_END);
        RTN_Close(rtn);
    }
    rtn = RTN_FindByName(img, "PinVerifyAfterCatch");
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforePinVerifyAfterCatch), IARG_END);
        RTN_Close(rtn);
    }
    rtn = RTN_FindByName(img, "PinVerifyInDestructor");
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforePinVerifyInDestructor), IARG_END);
        RTN_Close(rtn);
    }
}

ADDRINT ifPc;

ADDRINT IretIf(ADDRINT myPc)
{
    ifCalled                 = TRUE;
    ifPc                     = myPc;
    iretBeforeCalledAtIretIf = iretBeforeCalled;
    return (1);
}

VOID IretThen(ADDRINT myPc)
{
    thenCalled = TRUE;
    if (!ifCalled)
    {
        printf("***Error then instrumentation called before IretIf\n");
        exit(-1);
    }
    printf("IretThen ifPc %p  myPc %p\n", ifPc, myPc);
    if (ifPc != myPc)
    {
        printf("***Error unexpected PC\n");
        exit(-1);
    }
    fflush(stdout);
}

VOID IretAfter1()
{
    iretQAfter1Called = TRUE;
    if (!ifCalled)
    {
        printf("***Error IretAfter1 instrumentation called before IretIf\n");
        exit(-1);
    }
    if (!thenCalled)
    {
        printf("***Error IretAfter1 instrumentation called before IretThen\n");
        exit(-1);
    }

    printf("After1\n");
    fflush(stdout);
}

VOID IretAfter2()
{
    iretQAfter2Called = TRUE;
    if (!iretQAfter1Called)
    {
        printf("***Error IretAfter2 instrumentation called before IretAfter1\n");
        exit(-1);
    }

    printf("After2\n");
    fflush(stdout);
}

VOID IretBefore()
{
    iretBeforeCalled = TRUE;
    printf("IretBefore\n");
}

VOID Instruction(INS ins, VOID* v)
{
    if (INS_IsIRet(ins))
    {
        instrumentedIretq = TRUE;
        printf("instrumenting iret at %p\n", INS_Address(ins));
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IretBefore, IARG_END);
        INS_InsertIfCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)IretIf, IARG_INST_PTR, IARG_END);
        INS_InsertThenCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)IretThen, IARG_INST_PTR, IARG_END);
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)IretAfter1, IARG_END);
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)IretAfter2, IARG_END);
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    out = fopen(KnobOutputFile.Value().c_str(), "w");

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddContextChangeFunction(OnException, 0);

    IMG_AddInstrumentFunction(Image, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

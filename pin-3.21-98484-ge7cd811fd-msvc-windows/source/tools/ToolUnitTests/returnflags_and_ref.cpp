/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>

#include "pin.H"

THREADID myThread = INVALID_THREADID;

ADDRINT IfMyThread(THREADID threadId) { return threadId == myThread; }

// Test that using (IARG_REG_REFERENCE, REG_GAX) and (IARG_RETURN_REGS, REG_GFLAGS) to an anlysis function does not
// result in unexpected values in either register

ADDRINT SetGaxAndReturnGFlags(ADDRINT app_flags, ADDRINT gaxVal, ADDRINT* gaxRef)
{
    *gaxRef = gaxVal;
    return (app_flags);
}

int a[10];
int n = 10;

ADDRINT savedGaxVal, savedGFlagsVal;
ADDRINT SetGaxAndReturnGFlagsNoInline(ADDRINT app_flags, ADDRINT gaxVal, ADDRINT* gaxRef)
{
    for (int i = 0; i < n; i++)
    {
        a[i] = i;
    }
    *gaxRef        = gaxVal;
    savedGFlagsVal = app_flags;
    savedGaxVal    = gaxVal;
    return (app_flags);
}

ADDRINT SetGaxAndReturnGFlagsInline(ADDRINT app_flags, ADDRINT gaxVal, ADDRINT* gaxRef)
{
    *gaxRef        = gaxVal;
    savedGFlagsVal = app_flags;
    savedGaxVal    = gaxVal;
    return (app_flags);
}

void CompareGaxAndGFlags(ADDRINT app_flags, ADDRINT gaxVal)
{
    BOOL haveError = FALSE;
    if (savedGaxVal != gaxVal)
    {
        printf("Error in gax val\n");
        haveError = TRUE;
    }
    if (savedGFlagsVal != app_flags)
    {
        printf("Error in flags val\n");
        haveError = TRUE;
    }
    if (haveError)
    {
        exit(1);
    }
}

int instrumentationNum = 1;

VOID Instruction(INS ins, VOID* v)
{
    if (instrumentationNum & 0x1)
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(SetGaxAndReturnGFlagsNoInline), IARG_REG_VALUE, REG_GFLAGS, IARG_REG_VALUE,
                           REG_GAX, IARG_REG_REFERENCE, REG_GAX, IARG_RETURN_REGS, REG_GFLAGS, IARG_END);
    }

    else
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(SetGaxAndReturnGFlagsInline), IARG_REG_VALUE, REG_GFLAGS, IARG_REG_VALUE,
                           REG_GAX, IARG_REG_REFERENCE, REG_GAX, IARG_RETURN_REGS, REG_GFLAGS, IARG_END);
    }

    INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(CompareGaxAndGFlags), IARG_REG_VALUE, REG_GFLAGS, IARG_REG_VALUE, REG_GAX,
                       IARG_END);
    instrumentationNum++;
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (myThread == INVALID_THREADID)
    {
        myThread = threadid;
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    PIN_AddThreadStartFunction(ThreadStart, NULL);

    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 1;
}

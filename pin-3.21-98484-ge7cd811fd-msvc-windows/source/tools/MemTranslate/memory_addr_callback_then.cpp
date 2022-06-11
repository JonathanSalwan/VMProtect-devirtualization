/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::cout;
using std::endl;

BOOL ifTrue                 = FALSE;
BOOL foundTwoMemOpIns       = FALSE;
BOOL instrumentMemOp        = FALSE;
INT callbackCounter         = 0;
INT expectedCallbackCounter = 0;
THREADID mainThread         = INVALID_THREADID;

ADDRINT PIN_FAST_ANALYSIS_CALL memoryCallback(PIN_MEM_TRANS_INFO* memTransInfo, VOID* v)
{
    if (memTransInfo->flags.bits.isFromPin)
    {
        // PIN Internal memory dereference
        return memTransInfo->addr;
    }

    if (!ifTrue)
    {
        cout << "Callback shouldn't be excuted because ifRoutine returned false." << endl;
        PIN_ExitProcess(1);
    }

    ++callbackCounter;

    return memTransInfo->addr;
}

VOID thenMemoryCallbackCounter2(ADDRINT memeaOrig, ADDRINT memeaCallback, ADDRINT memeaOrig2, ADDRINT memeaCallback2)
{
    foundTwoMemOpIns = TRUE;
    ASSERTX(memeaOrig == memeaCallback);
    ASSERTX(memeaOrig2 == memeaCallback2);
    expectedCallbackCounter += 2;
}

VOID thenMemoryCallbackCounter(ADDRINT memeaOrig, ADDRINT memeaCallback)
{
    ASSERTX(memeaOrig == memeaCallback);
    ++expectedCallbackCounter;
}

ADDRINT ifRoutine(THREADID tid)
{
    //Win 10 apps isn't single threaded so need to filter the additional threads.
    if (tid != mainThread)
    {
        return FALSE;
    }

    ifTrue = !ifTrue;
    return ifTrue;
}

VOID Instruction(INS ins, VOID* v)
{
    if (instrumentMemOp)
    {
        UINT32 memOperands = INS_MemoryOperandCount(ins);
        if (memOperands)
        {
            if (1 == memOperands)
            {
                INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(ifRoutine), IARG_THREAD_ID, IARG_END);
                INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)thenMemoryCallbackCounter, IARG_MEMORYOP_EA, 0, IARG_MEMORYOP_PTR,
                                   0, IARG_END);
            }
            else if (2 == memOperands)
            {
                INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(ifRoutine), IARG_THREAD_ID, IARG_END);
                INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)thenMemoryCallbackCounter2, IARG_MEMORYOP_EA, 0,
                                   IARG_MEMORYOP_PTR, 0, IARG_MEMORYOP_EA, 1, IARG_MEMORYOP_PTR, 1, IARG_END);
            }
        }
    }
    else
    {
        if (INS_HasMemoryRead2(ins))
        {
            INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(ifRoutine), IARG_THREAD_ID, IARG_END);
            INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)thenMemoryCallbackCounter2, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_PTR,
                               IARG_MEMORYREAD2_EA, IARG_MEMORYREAD2_PTR, IARG_END);
        }
        else if (INS_IsMemoryWrite(ins) && INS_IsMemoryRead(ins))
        {
            INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(ifRoutine), IARG_THREAD_ID, IARG_END);
            INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)thenMemoryCallbackCounter2, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_PTR,
                               IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_PTR, IARG_END);
        }
        else if (INS_IsMemoryRead(ins))
        {
            INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(ifRoutine), IARG_THREAD_ID, IARG_END);
            INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)thenMemoryCallbackCounter, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_PTR,
                               IARG_END);
        }
        else if (INS_IsMemoryWrite(ins))
        {
            INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(ifRoutine), IARG_THREAD_ID, IARG_END);
            INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)thenMemoryCallbackCounter, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_PTR,
                               IARG_END);
        }
    }
    instrumentMemOp = !instrumentMemOp;
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (INVALID_THREADID == mainThread)
    {
        mainThread = threadid;
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (!foundTwoMemOpIns)
    {
        cout << "Didn't found two memory operands instruction." << endl;
        PIN_ExitProcess(1);
    }

    if (callbackCounter != expectedCallbackCounter)
    {
        cout << "Then-analysis routine executed different number of times than memory callback." << endl;
        cout << "callbackCounter: " << callbackCounter << ", "
             << "expectedCallbackCounter: " << expectedCallbackCounter << endl;
        PIN_ExitProcess(1);
    }
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, NULL);

    PIN_AddMemoryAddressTransFunction(memoryCallback, NULL);

    PIN_AddThreadStartFunction(ThreadStart, NULL);

    PIN_AddFiniFunction(Fini, NULL);

    PIN_StartProgram();

    return 1;
}

/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This tool demonstrates usage of the internal exception mechanism
 */

#include <stdio.h>
#include "pin.H"
#include <iostream>
#include <fstream>
using std::cerr;
using std::cout;
using std::endl;
using std::flush;

/* ===================================================================== */
/* Handlers                                                              */
/* ===================================================================== */

EXCEPT_HANDLING_RESULT GlobalHandler(THREADID threadIndex, EXCEPTION_INFO* pExceptInfo, PHYSICAL_CONTEXT* pPhysCtxt, VOID* v)
{
    cout << "GlobalHandler: Caught unexpected exception. " << PIN_ExceptionToString(pExceptInfo) << endl << flush;
    return EHR_UNHANDLED;
}

EXCEPT_HANDLING_RESULT DivideHandler(THREADID tid, EXCEPTION_INFO* pExceptInfo, PHYSICAL_CONTEXT* pPhysCtxt, VOID* appContextArg)
{
    if (PIN_GetExceptionCode(pExceptInfo) == EXCEPTCODE_INT_DIVIDE_BY_ZERO)
    {
        cout << "GlobalHandler: Caught divide by zero exception. " << PIN_ExceptionToString(pExceptInfo) << endl << flush;
        CONTEXT* appCtxt = (CONTEXT*)appContextArg;
        ADDRINT faultIp  = PIN_GetContextReg(appCtxt, REG_INST_PTR);
        PIN_SetExceptionAddress(pExceptInfo, faultIp);
        PIN_RaiseException(appCtxt, tid, pExceptInfo); //never returns
    }
    return EHR_CONTINUE_SEARCH;
}

/* ===================================================================== */
/* Analysis routine that emulates div                                    */
/* ===================================================================== */

VOID EmulateIntDivide(ADDRINT* pGdx, ADDRINT* pGax, ADDRINT divisor, CONTEXT* ctxt, THREADID tid)
{
    PIN_TryStart(tid, DivideHandler, ctxt);

    UINT64 dividend = *pGdx;
    dividend <<= 32;
    dividend += *pGax;
    *pGax = dividend / divisor;
    *pGdx = dividend % divisor;

    PIN_TryEnd(tid);
}

VOID EmulateMemDivide(ADDRINT* pGdx, ADDRINT* pGax, ADDRINT* pDivisor, unsigned int opSize, CONTEXT* ctxt, THREADID tid)
{
    ADDRINT divisor = 0;
    PIN_SafeCopy(&divisor, pDivisor, opSize);

    PIN_TryStart(tid, DivideHandler, ctxt);

    UINT64 dividend = *pGdx;
    dividend <<= 32;
    dividend += *pGax;
    *pGax = dividend / divisor;
    *pGdx = dividend % divisor;

    PIN_TryEnd(tid);
}

/* ===================================================================== */
/* Instrumentation routine that replaces div instruction                 */
/* ===================================================================== */

VOID InstrumentDivide(INS ins, VOID* v)
{
    if ((INS_Mnemonic(ins) == "DIV") && (INS_OperandIsReg(ins, 0)))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmulateIntDivide), IARG_REG_REFERENCE, REG_GDX, IARG_REG_REFERENCE, REG_GAX,
                       IARG_REG_VALUE, REG(INS_OperandReg(ins, 0)), IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if ((INS_Mnemonic(ins) == "DIV") && (!INS_OperandIsReg(ins, 0)))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmulateMemDivide), IARG_REG_REFERENCE, REG_GDX, IARG_REG_REFERENCE, REG_GAX,
                       IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool emulates divide" << endl;
    cerr << KNOB_BASE::StringKnobSummary() << endl << flush;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return Usage();
    INS_AddInstrumentFunction(InstrumentDivide, 0);
    PIN_AddInternalExceptionHandler(GlobalHandler, NULL);
    PIN_StartProgram(); // Never returns
    return 0;
}

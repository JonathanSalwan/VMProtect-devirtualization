/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This is a test of the PIN_RaiseException() API and is closely connected to the
 * "faultcheck" test.  The "faultcheck" test has a series of assembly functions
 * that raise each type of exception and the application establishes a signal
 * handler to catch and print each exception.
 *
 * This tool is meant to run only on the "faultcheck" test application.  The tool
 * uses INS_Delete() to delete each of the faulty instructions and replaces each
 * with a call to an analysis routine that uses PIN_RaiseException() to raise
 * exactly the same exception.  We then run the "faultcheck" test twice, once
 * natively and once with this tool, and compare the output from the two runs.
 * This ensures that Pin's emulated exceptions exactly match the exceptions raised
 * by the real O/S.
 */

#include <iostream>
#include <cstdlib>
#include "pin.H"
#include "raise-exception-addrs.h"

KNOB< BOOL > KnobUseIargConstContext(KNOB_MODE_WRITEONCE, "pintool", "const_context", "0", "use IARG_CONST_CONTEXT");

/*
 * Bit positions in the MXCSR and x87 status registers that correspond to
 * each exception condition.
 */
enum
{
    EXCBIT_IE = (1 << 0),
    EXCBIT_DE = (1 << 1),
    EXCBIT_ZE = (1 << 2),
    EXCBIT_OE = (1 << 3),
    EXCBIT_UE = (1 << 4),
    EXCBIT_PE = (1 << 5)
};

static VOID OnImage(IMG, VOID*);
static VOID OnInstruction(INS, VOID*);
static void OnFini(INT32, VOID*);
static void AtSetLabels(RAISE_EXCEPTION_ADDRS*);
static void DoUnmappedRead(CONTEXT*, THREADID, ADDRINT);
static void DoUnmappedWrite(CONTEXT*, THREADID, ADDRINT);
static void DoInaccessibleRead(CONTEXT*, THREADID, ADDRINT);
static void DoInaccessibleWrite(CONTEXT*, THREADID, ADDRINT);
static void DoMisalignedRead(CONTEXT*, THREADID);
static void DoMisalignedWrite(CONTEXT*, THREADID);
static void DoIllegalInstruction(CONTEXT*, THREADID);
static void DoPrivilegedInstruction(CONTEXT*, THREADID);
static void DoIntegerDivideByZero(CONTEXT*, THREADID);
static void DoIntegerOverflowTrap(CONTEXT*, THREADID, UINT32);
static void DoBoundTrap(CONTEXT*, THREADID, UINT32);
static void DoX87DivideByZero(CONTEXT*, THREADID);
static void DoX87Overflow(CONTEXT*, THREADID);
static void DoX87Underflow(CONTEXT*, THREADID);
static void DoX87Precision(CONTEXT*, THREADID);
static void DoX87InvalidOperation(CONTEXT*, THREADID);
static void DoX87DenormalizedOperand(CONTEXT*, THREADID);
static void DoX87StackUnderflow(CONTEXT*, THREADID);
static void DoX87StackOverflow(CONTEXT*, THREADID);
static void DoSIMDDivideByZero(CONTEXT*, THREADID);
static void DoSIMDOverflow(CONTEXT*, THREADID);
static void DoSIMDUnderflow(CONTEXT*, THREADID);
static void DoSIMDPrecision(CONTEXT*, THREADID);
static void DoSIMDInvalidOperation(CONTEXT*, THREADID);
static void DoSIMDDenormalizedOperand(CONTEXT*, THREADID);
static void DoBreakpointTrap(CONTEXT*, THREADID, UINT32);

BOOL HaveExceptionAddrs            = FALSE;
BOOL ExpectUnmappedRead            = FALSE;
BOOL ExpectUnmappedWrite           = FALSE;
BOOL ExpectInaccessibleRead        = FALSE;
BOOL ExpectInaccessibleWrite       = FALSE;
BOOL ExpectMisalignedRead          = FALSE;
BOOL ExpectMisalignedWrite         = FALSE;
BOOL ExpectIllegalInstruction      = FALSE;
BOOL ExpectPrivilegedInstruction   = FALSE;
BOOL ExpectIntegerDivideByZero     = FALSE;
BOOL ExpectIntegerOverflowTrap     = FALSE;
BOOL ExpectBoundTrap               = FALSE;
BOOL ExpectX87DivideByZero         = FALSE;
BOOL ExpectX87Overflow             = FALSE;
BOOL ExpectX87Underflow            = FALSE;
BOOL ExpectX87Precision            = FALSE;
BOOL ExpectX87InvalidOperation     = FALSE;
BOOL ExpectX87DenormalizedOperand  = FALSE;
BOOL ExpectX87StackUnderflow       = FALSE;
BOOL ExpectX87StackOverflow        = FALSE;
BOOL ExpectSimdDivideByZero        = FALSE;
BOOL ExpectSimdOverflow            = FALSE;
BOOL ExpectSimdUnderflow           = FALSE;
BOOL ExpectSimdPrecision           = FALSE;
BOOL ExpectSimdInvalidOperation    = FALSE;
BOOL ExpectSimdDenormalizedOperand = FALSE;
BOOL ExpectBreakpointTrap          = FALSE;

RAISE_EXCEPTION_ADDRS ExceptionAddrs;

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    IMG_AddInstrumentFunction(OnImage, 0);
    INS_AddInstrumentFunction(OnInstruction, 0);
    PIN_AddFiniFunction(OnFini, 0);

    PIN_StartProgram();
    return 0;
}

static VOID OnImage(IMG img, VOID*)
{
    RTN rtn = RTN_FindByName(img, "SetLabelsForPinTool");
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(AtSetLabels), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        RTN_Close(rtn);
    }
}

static VOID OnInstruction(INS ins, VOID*)
{
    if (!HaveExceptionAddrs) return;

    char* insAddr = reinterpret_cast< char* >(INS_Address(ins));
    if (insAddr == ExceptionAddrs._unmappedRead)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoUnmappedRead), (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT,
                       IARG_THREAD_ID, IARG_ADDRINT, ExceptionAddrs._unmappedReadAddr, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._unmappedWrite)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoUnmappedWrite),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_ADDRINT,
                       ExceptionAddrs._unmappedWriteAddr, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._inaccessibleRead)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoInaccessibleRead),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_ADDRINT,
                       ExceptionAddrs._inaccessibleReadAddr, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._inaccessibleWrite)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoInaccessibleWrite),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_ADDRINT,
                       ExceptionAddrs._inaccessibleWriteAddr, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._misalignedRead)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoMisalignedRead),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._misalignedWrite)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoMisalignedWrite),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._illegalInstruction)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoIllegalInstruction),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._privilegedInstruction)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoPrivilegedInstruction),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._integerDivideByZero)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoIntegerDivideByZero),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._integerOverflowTrap)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoIntegerOverflowTrap),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_UINT32,
                       static_cast< UINT32 >(INS_Size(ins)), IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._boundTrap)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoBoundTrap), (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT,
                       IARG_THREAD_ID, IARG_UINT32, static_cast< UINT32 >(INS_Size(ins)), IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._x87DivideByZero)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoX87DivideByZero),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._x87Overflow)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoX87Overflow), (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT,
                       IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._x87Underflow)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoX87Underflow), (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT,
                       IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._x87Precision)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoX87Precision), (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT,
                       IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._x87InvalidOperation)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoX87InvalidOperation),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._x87DenormalizedOperand)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoX87DenormalizedOperand),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._x87StackUnderflow)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoX87StackUnderflow),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._x87StackOverflow)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoX87StackOverflow),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._simdDivideByZero)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoSIMDDivideByZero),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._simdOverflow)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoSIMDOverflow), (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT,
                       IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._simdUnderflow)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoSIMDUnderflow),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._simdPrecision)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoSIMDPrecision),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._simdInvalidOperation)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoSIMDInvalidOperation),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._simdDenormalizedOperand)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoSIMDDenormalizedOperand),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_Delete(ins);
    }
    if (insAddr == ExceptionAddrs._breakpointTrap)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoBreakpointTrap),
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_UINT32,
                       static_cast< UINT32 >(INS_Size(ins)), IARG_END);
        INS_Delete(ins);
    }
}

static void OnFini(INT32, VOID*)
{
    if (!HaveExceptionAddrs)
    {
        std::cerr << "Did not get exception lables\n";
        std::exit(1);
    }
    if (ExpectUnmappedRead)
    {
        std::cerr << "Did not emulate unmapped read\n";
        std::exit(1);
    }
    if (ExpectUnmappedWrite)
    {
        std::cerr << "Did not emulate unmapped write\n";
        std::exit(1);
    }
    if (ExpectInaccessibleRead)
    {
        std::cerr << "Did not emulate inaccessible read\n";
        std::exit(1);
    }
    if (ExpectInaccessibleWrite)
    {
        std::cerr << "Did not emulate inaccessible write\n";
        std::exit(1);
    }
    if (ExpectMisalignedRead)
    {
        std::cerr << "Did not emulate misaligned read\n";
        std::exit(1);
    }
    if (ExpectMisalignedWrite)
    {
        std::cerr << "Did not emulate misaligned write\n";
        std::exit(1);
    }
    if (ExpectIllegalInstruction)
    {
        std::cerr << "Did not emulate illegal instruction\n";
        std::exit(1);
    }
    if (ExpectPrivilegedInstruction)
    {
        std::cerr << "Did not emulate privileged instruction\n";
        std::exit(1);
    }
    if (ExpectIntegerDivideByZero)
    {
        std::cerr << "Did not emulate integer divide by zero\n";
        std::exit(1);
    }
    if (ExpectIntegerOverflowTrap)
    {
        std::cerr << "Did not emulate integer overflow trap\n";
        std::exit(1);
    }
    if (ExpectBoundTrap)
    {
        std::cerr << "Did not emulate bound trap\n";
        std::exit(1);
    }
    if (ExpectX87DivideByZero)
    {
        std::cerr << "Did not emulate x87 divide by zero\n";
        std::exit(1);
    }
    if (ExpectX87Overflow)
    {
        std::cerr << "Did not emulate x87 overflow\n";
        std::exit(1);
    }
    if (ExpectX87Underflow)
    {
        std::cerr << "Did not emulate x87 underflow\n";
        std::exit(1);
    }
    if (ExpectX87Precision)
    {
        std::cerr << "Did not emulate x87 precision\n";
        std::exit(1);
    }
    if (ExpectX87InvalidOperation)
    {
        std::cerr << "Did not emulate x87 invalid operation\n";
        std::exit(1);
    }
    if (ExpectX87DenormalizedOperand)
    {
        std::cerr << "Did not emulate x87 denormalized operand\n";
        std::exit(1);
    }
    if (ExpectX87StackUnderflow)
    {
        std::cerr << "Did not emulate x87 stack underflow\n";
        std::exit(1);
    }
    if (ExpectX87StackOverflow)
    {
        std::cerr << "Did not emulate x87 stack overflow\n";
        std::exit(1);
    }
    if (ExpectSimdDivideByZero)
    {
        std::cerr << "Did not emulate simd divide by zero\n";
        std::exit(1);
    }
    if (ExpectSimdOverflow)
    {
        std::cerr << "Did not emulate simd overflow\n";
        std::exit(1);
    }
    if (ExpectSimdUnderflow)
    {
        std::cerr << "Did not emulate simd underflow\n";
        std::exit(1);
    }
    if (ExpectSimdPrecision)
    {
        std::cerr << "Did not emulate simd precision\n";
        std::exit(1);
    }
    if (ExpectSimdInvalidOperation)
    {
        std::cerr << "Did not emulate simd invalid operation\n";
        std::exit(1);
    }
    if (ExpectSimdDenormalizedOperand)
    {
        std::cerr << "Did not emulate simd denormalized operand\n";
        std::exit(1);
    }
    if (ExpectBreakpointTrap)
    {
        std::cerr << "Did not emulate breakpoint trap\n";
        std::exit(1);
    }
}

static void AtSetLabels(RAISE_EXCEPTION_ADDRS* exceptionAddrs)
{
    size_t sz = PIN_SafeCopy(&ExceptionAddrs, exceptionAddrs, sizeof(ExceptionAddrs));
    if (sz != sizeof(ExceptionAddrs))
    {
        std::cerr << "Unable to copy RAISE_EXCEPTION_ADDRS\n";
        std::exit(1);
    }

    HaveExceptionAddrs            = TRUE;
    ExpectUnmappedRead            = (ExceptionAddrs._unmappedRead != 0);
    ExpectUnmappedWrite           = (ExceptionAddrs._unmappedWrite != 0);
    ExpectInaccessibleRead        = (ExceptionAddrs._inaccessibleRead != 0);
    ExpectInaccessibleWrite       = (ExceptionAddrs._inaccessibleWrite != 0);
    ExpectMisalignedRead          = (ExceptionAddrs._misalignedRead != 0);
    ExpectMisalignedWrite         = (ExceptionAddrs._misalignedWrite != 0);
    ExpectIllegalInstruction      = (ExceptionAddrs._illegalInstruction != 0);
    ExpectPrivilegedInstruction   = (ExceptionAddrs._privilegedInstruction != 0);
    ExpectIntegerDivideByZero     = (ExceptionAddrs._integerDivideByZero != 0);
    ExpectIntegerOverflowTrap     = (ExceptionAddrs._integerOverflowTrap != 0);
    ExpectBoundTrap               = (ExceptionAddrs._boundTrap != 0);
    ExpectX87DivideByZero         = (ExceptionAddrs._x87DivideByZero != 0);
    ExpectX87Overflow             = (ExceptionAddrs._x87Overflow != 0);
    ExpectX87Underflow            = (ExceptionAddrs._x87Underflow != 0);
    ExpectX87Precision            = (ExceptionAddrs._x87Precision != 0);
    ExpectX87InvalidOperation     = (ExceptionAddrs._x87InvalidOperation != 0);
    ExpectX87DenormalizedOperand  = (ExceptionAddrs._x87DenormalizedOperand != 0);
    ExpectX87StackUnderflow       = (ExceptionAddrs._x87StackUnderflow != 0);
    ExpectX87StackOverflow        = (ExceptionAddrs._x87StackOverflow != 0);
    ExpectSimdDivideByZero        = (ExceptionAddrs._simdDivideByZero != 0);
    ExpectSimdOverflow            = (ExceptionAddrs._simdOverflow != 0);
    ExpectSimdUnderflow           = (ExceptionAddrs._simdUnderflow != 0);
    ExpectSimdPrecision           = (ExceptionAddrs._simdPrecision != 0);
    ExpectSimdInvalidOperation    = (ExceptionAddrs._simdInvalidOperation != 0);
    ExpectSimdDenormalizedOperand = (ExceptionAddrs._simdDenormalizedOperand != 0);
    ExpectBreakpointTrap          = (ExceptionAddrs._breakpointTrap != 0);

    PIN_RemoveInstrumentation();
}

static void DoUnmappedRead(CONTEXT* ctxt, THREADID tid, ADDRINT accessAddr)
{
    ExpectUnmappedRead = FALSE;
    ADDRINT pc         = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitAccessFaultInfo(&exc, EXCEPTCODE_ACCESS_INVALID_ADDRESS, pc, accessAddr, FAULTY_ACCESS_READ);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoUnmappedWrite(CONTEXT* ctxt, THREADID tid, ADDRINT accessAddr)
{
    ExpectUnmappedWrite = FALSE;
    ADDRINT pc          = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitAccessFaultInfo(&exc, EXCEPTCODE_ACCESS_INVALID_ADDRESS, pc, accessAddr, FAULTY_ACCESS_WRITE);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoInaccessibleRead(CONTEXT* ctxt, THREADID tid, ADDRINT accessAddr)
{
    ExpectInaccessibleRead = FALSE;
    ADDRINT pc             = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitAccessFaultInfo(&exc, EXCEPTCODE_ACCESS_DENIED, pc, accessAddr, FAULTY_ACCESS_READ);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoInaccessibleWrite(CONTEXT* ctxt, THREADID tid, ADDRINT accessAddr)
{
    ExpectInaccessibleWrite = FALSE;
    ADDRINT pc              = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitAccessFaultInfo(&exc, EXCEPTCODE_ACCESS_DENIED, pc, accessAddr, FAULTY_ACCESS_WRITE);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoMisalignedRead(CONTEXT* ctxt, THREADID tid)
{
    ExpectMisalignedRead = FALSE;
    ADDRINT pc           = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_ACCESS_MISALIGNED, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoMisalignedWrite(CONTEXT* ctxt, THREADID tid)
{
    ExpectMisalignedWrite = FALSE;
    ADDRINT pc            = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_ACCESS_MISALIGNED, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoIllegalInstruction(CONTEXT* ctxt, THREADID tid)
{
    ExpectIllegalInstruction = FALSE;
    ADDRINT pc               = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_ILLEGAL_INS, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoPrivilegedInstruction(CONTEXT* ctxt, THREADID tid)
{
    ExpectPrivilegedInstruction = FALSE;
    ADDRINT pc                  = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_PRIVILEGED_INS, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoIntegerDivideByZero(CONTEXT* ctxt, THREADID tid)
{
    ExpectIntegerDivideByZero = FALSE;
    ADDRINT pc                = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_INT_DIVIDE_BY_ZERO, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoIntegerOverflowTrap(CONTEXT* context, THREADID tid, UINT32 instSize)
{
    ExpectIntegerOverflowTrap = FALSE;
    ADDRINT pc                = PIN_GetContextReg(context, REG_INST_PTR);

    CONTEXT writableContext, *ctxt;
    if (KnobUseIargConstContext)
    { // need to copy the ctxt into a writable context
        PIN_SaveContext(context, &writableContext);
        ctxt = &writableContext;
    }
    else
    {
        ctxt = context;
    }

    PIN_SetContextReg(ctxt, REG_INST_PTR, pc + instSize); // The fault is reported on the PC after the trap instruction.
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_INT_OVERFLOW_TRAP, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoBoundTrap(CONTEXT* ctxt, THREADID tid, UINT32 instSize)
{
    ExpectBoundTrap = FALSE;
    ADDRINT pc      = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_INT_BOUNDS_EXCEEDED, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoX87DivideByZero(CONTEXT* ctxt, THREADID tid)
{
    ExpectX87DivideByZero = FALSE;
    ADDRINT pc            = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_X87_DIVIDE_BY_ZERO, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoX87Overflow(CONTEXT* ctxt, THREADID tid)
{
    ExpectX87Overflow = FALSE;
    ADDRINT pc        = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_X87_OVERFLOW, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoX87Underflow(CONTEXT* ctxt, THREADID tid)
{
    ExpectX87Underflow = FALSE;
    ADDRINT pc         = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_X87_UNDERFLOW, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoX87Precision(CONTEXT* ctxt, THREADID tid)
{
    ExpectX87Precision = FALSE;
    ADDRINT pc         = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_X87_INEXACT_RESULT, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoX87InvalidOperation(CONTEXT* ctxt, THREADID tid)
{
    ExpectX87InvalidOperation = FALSE;
    ADDRINT pc                = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_X87_INVALID_OPERATION, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoX87DenormalizedOperand(CONTEXT* ctxt, THREADID tid)
{
    ExpectX87DenormalizedOperand = FALSE;
    ADDRINT pc                   = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_X87_DENORMAL_OPERAND, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoX87StackUnderflow(CONTEXT* ctxt, THREADID tid)
{
    ExpectX87StackUnderflow = FALSE;
    ADDRINT pc              = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_X87_STACK_ERROR, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoX87StackOverflow(CONTEXT* ctxt, THREADID tid)
{
    ExpectX87StackOverflow = FALSE;
    ADDRINT pc             = PIN_GetContextReg(ctxt, REG_INST_PTR);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_X87_STACK_ERROR, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoSIMDDivideByZero(CONTEXT* context, THREADID tid)
{
    ExpectSimdDivideByZero = FALSE;
    ADDRINT pc             = PIN_GetContextReg(context, REG_INST_PTR);
    ADDRINT mxcsr          = PIN_GetContextReg(context, REG_MXCSR);

    CONTEXT writableContext, *ctxt;
    if (KnobUseIargConstContext)
    { // need to copy the ctxt into a writable context
        PIN_SaveContext(context, &writableContext);
        ctxt = &writableContext;
    }
    else
    {
        ctxt = context;
    }

    PIN_SetContextReg(ctxt, REG_MXCSR, mxcsr | EXCBIT_ZE);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_SIMD_DIVIDE_BY_ZERO, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoSIMDOverflow(CONTEXT* context, THREADID tid)
{
    ExpectSimdOverflow = FALSE;
    ADDRINT pc         = PIN_GetContextReg(context, REG_INST_PTR);
    ADDRINT mxcsr      = PIN_GetContextReg(context, REG_MXCSR);

    CONTEXT writableContext, *ctxt;
    if (KnobUseIargConstContext)
    { // need to copy the ctxt into a writable context
        PIN_SaveContext(context, &writableContext);
        ctxt = &writableContext;
    }
    else
    {
        ctxt = context;
    }

    PIN_SetContextReg(ctxt, REG_MXCSR, mxcsr | (EXCBIT_OE | EXCBIT_PE));
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_SIMD_OVERFLOW, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoSIMDUnderflow(CONTEXT* context, THREADID tid)
{
    ExpectSimdUnderflow = FALSE;
    ADDRINT pc          = PIN_GetContextReg(context, REG_INST_PTR);
    ADDRINT mxcsr       = PIN_GetContextReg(context, REG_MXCSR);

    CONTEXT writableContext, *ctxt;
    if (KnobUseIargConstContext)
    { // need to copy the ctxt into a writable context
        PIN_SaveContext(context, &writableContext);
        ctxt = &writableContext;
    }
    else
    {
        ctxt = context;
    }

    PIN_SetContextReg(ctxt, REG_MXCSR, mxcsr | (EXCBIT_UE | EXCBIT_PE));
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_SIMD_UNDERFLOW, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoSIMDPrecision(CONTEXT* context, THREADID tid)
{
    ExpectSimdPrecision = FALSE;
    ADDRINT pc          = PIN_GetContextReg(context, REG_INST_PTR);
    ADDRINT mxcsr       = PIN_GetContextReg(context, REG_MXCSR);

    CONTEXT writableContext, *ctxt;
    if (KnobUseIargConstContext)
    { // need to copy the ctxt into a writable context
        PIN_SaveContext(context, &writableContext);
        ctxt = &writableContext;
    }
    else
    {
        ctxt = context;
    }

    PIN_SetContextReg(ctxt, REG_MXCSR, mxcsr | EXCBIT_PE);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_SIMD_INEXACT_RESULT, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoSIMDInvalidOperation(CONTEXT* context, THREADID tid)
{
    ExpectSimdInvalidOperation = FALSE;
    ADDRINT pc                 = PIN_GetContextReg(context, REG_INST_PTR);
    ADDRINT mxcsr              = PIN_GetContextReg(context, REG_MXCSR);

    CONTEXT writableContext, *ctxt;
    if (KnobUseIargConstContext)
    { // need to copy the ctxt into a writable context
        PIN_SaveContext(context, &writableContext);
        ctxt = &writableContext;
    }
    else
    {
        ctxt = context;
    }

    PIN_SetContextReg(ctxt, REG_MXCSR, mxcsr | EXCBIT_IE);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_SIMD_INVALID_OPERATION, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoSIMDDenormalizedOperand(CONTEXT* context, THREADID tid)
{
    ExpectSimdDenormalizedOperand = FALSE;
    ADDRINT pc                    = PIN_GetContextReg(context, REG_INST_PTR);
    ADDRINT mxcsr                 = PIN_GetContextReg(context, REG_MXCSR);

    CONTEXT writableContext, *ctxt;
    if (KnobUseIargConstContext)
    { // need to copy the ctxt into a writable context
        PIN_SaveContext(context, &writableContext);
        ctxt = &writableContext;
    }
    else
    {
        ctxt = context;
    }

    PIN_SetContextReg(ctxt, REG_MXCSR, mxcsr | EXCBIT_DE);
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_SIMD_DENORMAL_OPERAND, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

static void DoBreakpointTrap(CONTEXT* context, THREADID tid, UINT32 instSize)
{
    ExpectBreakpointTrap = FALSE;
    ADDRINT pc           = PIN_GetContextReg(context, REG_INST_PTR);

    CONTEXT writableContext, *ctxt;
    if (KnobUseIargConstContext)
    { // need to copy the ctxt into a writable context
        PIN_SaveContext(context, &writableContext);
        ctxt = &writableContext;
    }
    else
    {
        ctxt = context;
    }

    PIN_SetContextReg(ctxt, REG_INST_PTR, pc + instSize); // The fault is reported on the PC after the trap instruction.
    EXCEPTION_INFO exc;
    PIN_InitExceptionInfo(&exc, EXCEPTCODE_DBG_BREAKPOINT_TRAP, pc);
    PIN_RaiseException(ctxt, tid, &exc);
}

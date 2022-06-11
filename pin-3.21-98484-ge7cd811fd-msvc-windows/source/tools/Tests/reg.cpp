/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "pin.H"
using std::endl;
using std::hex;
using std::ofstream;

ofstream out("reg.out");

UINT64 icount = 0;

VOID docount(VOID* ip, VOID* reg)
{
    icount++;
    if ((icount % 1000) == 1) out << "ip:" << ip << " count:" << icount << " SP:" << reg << endl;
}

VOID Instruction(INS ins, VOID* v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_INST_PTR, IARG_REG_VALUE, REG_STACK_PTR, IARG_END);
}

VOID PrintError(ADDRINT ip, ADDRINT al, ADDRINT ah, ADDRINT ax, ADDRINT eax, ADDRINT rax)
{
    std::cerr << hex << "ip " << ip << " al " << al << " ah " << ah << " ax " << ax << " eax " << eax << " rax " << rax << endl;
    exit(1);
}

VOID CheckRegs(ADDRINT ip, ADDRINT al, ADDRINT ah, ADDRINT ax, ADDRINT eax, ADDRINT rax)
{
    if ((eax & 0xff) != al)
    {
        PrintError(ip, al, ah, ax, eax, rax);
    }

    if (((eax & 0xff00) >> 8) != ah)
    {
        PrintError(ip, al, ah, ax, eax, rax);
    }

    if ((eax & 0xffff) != ax)
    {
        PrintError(ip, al, ah, ax, eax, rax);
    }

#if defined(TARGET_IA32E)
    if ((rax & 0xffffffff) != eax)
    {
        PrintError(ip, al, ah, ax, eax, rax);
    }
#endif
}

VOID CheckArRegs(ADDRINT rsc, ADDRINT bsp, ADDRINT bspstore, ADDRINT rnat, ADDRINT fcr, ADDRINT eflag, ADDRINT csd, ADDRINT ssd,
                 ADDRINT cflg, ADDRINT fsr, ADDRINT fir, ADDRINT fdr, ADDRINT ccv, ADDRINT unat, ADDRINT fpsr, ADDRINT itc,
                 ADDRINT pfs, ADDRINT lc, ADDRINT ec)
{}

VOID CheckNat(ADDRINT nat) {}

ADDRINT capturedVal;
ADDRINT capturedConstVal;

// Make it inlineable
VOID CaptureRef(ADDRINT* ref, ADDRINT* constRef)
{
    capturedVal      = *ref;
    capturedConstVal = *constRef;
}

VOID CheckRef(char const* name, ADDRINT val, ADDRINT* ref, ADDRINT const* constRef)
{
    if (val != capturedVal || val != *ref || val != capturedConstVal || val != *constRef)
    {
        std::cerr << hex << name << " val: " << val << " ref: " << *ref << " constRef: " << *constRef << endl;
        exit(1);
    }
}

VOID InsertCheckRef(TRACE trace, char const* name, REG reg)
{
    // This is inlineable
    TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(CaptureRef), IARG_REG_REFERENCE, reg, IARG_REG_CONST_REFERENCE, reg, IARG_END);
    // This is NOT inlineable
    TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(CheckRef), IARG_PTR, name, IARG_REG_VALUE, reg, IARG_REG_REFERENCE, reg,
                     IARG_REG_CONST_REFERENCE, reg, IARG_END);
}

VOID Trace(TRACE trace, VOID* v)
{
    static BOOL first = TRUE;

#if defined(TARGET_IA32) || defined(TARGET_IA32E)

    if (first)
    {
        InsertCheckRef(trace, "gax", REG_GAX);
        InsertCheckRef(trace, "stack", REG_STACK_PTR);
    }

    TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(CheckRegs), IARG_INST_PTR, IARG_REG_VALUE, REG_AL, IARG_REG_VALUE, REG_AH,
                     IARG_REG_VALUE, REG_AX, IARG_REG_VALUE, REG_EAX,
#if defined(TARGET_IA32E)
                     IARG_REG_VALUE, REG_RAX,
#endif
                     IARG_END);

    INS ins = BBL_InsHead(TRACE_BblHead(trace));

    static INT32 origCount = 0;
    if (INS_IsOriginal(ins))
    {
        origCount++;
    }

    static INT32 immcount = 10;
    if (immcount > 0)
    {
        for (UINT32 op = 0; op < INS_OperandCount(ins); op++)
        {
            if (INS_OperandIsImmediate(ins, op))
            {
                std::cerr << "Immediate: " << hex << INS_OperandImmediate(ins, op) << " ins " << INS_Disassemble(ins) << endl;
                immcount--;
            }
        }
    }
#endif

    first = FALSE;
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);
    TRACE_AddInstrumentFunction(Trace, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

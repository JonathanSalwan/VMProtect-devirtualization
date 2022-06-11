/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
extern "C" void DoXmm();
/* The function DoXmm is 
DoXmm:
    sub         $0x3c, %esp
    movdqa      %xmm2, (%esp)
	add         $0x3c, %esp
    ret

This is a non-windows test, The DoXmm function assumes that when called the esp is aligned on 12mod16 
and when used in the movdqa instruction it is aligned on 16. 
This test verifies that Pin maintains this alignment when inlining the function - i.e. no misalignment
fault occurs
*/

VOID Instruction(INS ins, VOID* v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)DoXmm, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)DoXmm, IARG_UINT32, 1, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)DoXmm, IARG_UINT32, 1, IARG_UINT32, 2, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)DoXmm, IARG_UINT32, 1, IARG_UINT32, 2, IARG_UINT32, 3, IARG_END);

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)DoXmm, IARG_REG_REFERENCE, REG_EAX, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)DoXmm, IARG_REG_REFERENCE, REG_EAX, IARG_UINT32, 1, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)DoXmm, IARG_REG_REFERENCE, REG_EAX, IARG_UINT32, 1, IARG_UINT32, 2, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)DoXmm, IARG_REG_REFERENCE, REG_EAX, IARG_UINT32, 1, IARG_UINT32, 2, IARG_UINT32,
                   3, IARG_END);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_StartProgram();
    return 0;
}

/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include "pin.H"

ADDRINT capturedVal;
ADDRINT capturedConstVal;
ADDRINT capturedRegEspBefore;

BOOL badEsp = FALSE;

// Make it inlineable
ADDRINT CaptureRefWithReturnReg(ADDRINT* ref, ADDRINT* constRef)
{
    capturedVal      = *ref;
    capturedConstVal = *constRef;
    return (*constRef);
}

VOID CaptureRef(ADDRINT* ref, ADDRINT* constRef)
{
    capturedVal      = *ref;
    capturedConstVal = *constRef;
}

VOID CaptureEspBefore(ADDRINT regEsp) { capturedRegEspBefore = regEsp; }

int haveBadEsp;
VOID CaptureEspAfter(ADDRINT regEsp)
{
    haveBadEsp = (regEsp != capturedRegEspBefore);
    badEsp |= haveBadEsp;
}

ADDRINT imgStartAdd;
USIZE imgSize;

VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        imgStartAdd = IMG_StartAddress(img);
        imgSize     = IMG_SizeMapped(img);
    }
}

VOID Instruction(INS ins, VOID* v)
{
    //instrument if ins is app instruction
    if (INS_Address(ins) >= imgStartAdd && INS_Address(ins) < (imgStartAdd + imgSize))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CaptureEspBefore), IARG_REG_VALUE, REG_STACK_PTR, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CaptureRefWithReturnReg), IARG_REG_REFERENCE, REG_GAX,
                       IARG_REG_CONST_REFERENCE, REG_GAX, IARG_RETURN_REGS, REG_GAX, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CaptureRef), IARG_REG_REFERENCE, REG_GAX, IARG_REG_CONST_REFERENCE, REG_GAX,
                       IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CaptureEspAfter), IARG_REG_VALUE, REG_STACK_PTR, IARG_END);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (badEsp)
    {
        printf("***ERROR is esp value\n");
        exit(-1);
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, NULL);

    PIN_AddFiniFunction(Fini, NULL);

    IMG_AddInstrumentFunction(ImageLoad, NULL);

    // Never returns
    PIN_StartProgram();

    return 1;
}

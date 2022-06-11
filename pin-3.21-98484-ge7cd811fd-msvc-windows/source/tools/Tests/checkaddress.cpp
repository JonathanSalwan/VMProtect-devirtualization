/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"
#include <iostream>

UINT32* lastEa;
UINT32 lastValue;

VOID CaptureEaAndValue(UINT32* ea)
{
    lastEa    = ea;
    lastValue = *ea;
}

VOID TestValue(VOID* ip, UINT32 val)
{
    if (val != lastValue)
    {
        fprintf(stderr, "Difference IP: %p, EA: %p, register value: %x, memory value: %x\n", ip, lastEa, val, lastValue);

        exit(1);
    }
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

ADDRINT IsMainExe(ADDRINT InsAdd)
{
    //instrument if ins is app instruction
    return (InsAdd >= imgStartAdd && InsAdd < (imgStartAdd + imgSize));
}

VOID Instruction(INS ins, VOID* v)
{
    if (IsMainExe(INS_Address(ins)) && INS_Mnemonic(ins) == "MOV" && INS_IsMemoryRead(ins) && REG_is_gr(INS_RegW(ins, 0)))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CaptureEaAndValue), IARG_MEMORYREAD_EA, IARG_END);
        INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(TestValue), IARG_INST_PTR, IARG_REG_VALUE, INS_RegW(ins, 0), IARG_END);
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    IMG_AddInstrumentFunction(ImageLoad, NULL);
    INS_AddInstrumentFunction(Instruction, NULL);

    // Never returns
    PIN_StartProgram();

    return 1;
}

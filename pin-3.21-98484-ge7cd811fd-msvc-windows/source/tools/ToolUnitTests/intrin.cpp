/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"
#include <iostream>
#include <fstream>
#include <mmintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>

void convert_dword_to_double(double* result, UINT32 i)
{
    __m128d mmin, mmout;
    mmin  = _mm_setzero_pd();
    mmout = _mm_cvtsi32_sd(mmin, i);
    _mm_store_sd(result, mmout);
}

void convert(ADDRINT memea)
{
    double d; // uses XMM, not X87 for data
    UINT32 i = (UINT32)memea;
    convert_dword_to_double(&d, i);
}

void convert_load_addr(INS ins, void* v)
{
    if (INS_Opcode(ins) == XED_ICLASS_MOV && INS_IsMemoryRead(ins) && INS_OperandIsReg(ins, 0) && INS_OperandIsMemory(ins, 1))
    {
        // op0 <- *op1
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(convert), IARG_MEMORYREAD_EA, IARG_END);
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    INS_AddInstrumentFunction(convert_load_addr, 0);
    PIN_StartProgram(); // Never returns
    return 0;
}

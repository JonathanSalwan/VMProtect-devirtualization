/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <iomanip>
#include <cstdlib> // for atoi w/gcc4.3.x

extern "C" void MyMemCpy();

unsigned char src[128];
unsigned char dst[128];

using std::cout;
using std::endl;

VOID CheckCopyAndReset(unsigned char* src, unsigned char* dst)
{
    for (int i = 0; i < 128; i++)
    {
        if (src[i] != i)
        {
            cout << "***Error in copy of src to dst, src changed" << endl;
            exit(-1);
        }
        if (dst[i] != i)
        {
            cout << "***Error in tool copy of src to dst, unexpected value in dst" << endl;
            exit(-1);
        }
    }
    for (int i = 0; i < 128; i++)
    {
        src[i] = i;
        dst[i] = (i + 1) & 0xff;
    }
    for (int i = 0; i < 128; i++)
    {
        if (src[i] != i)
        {
            cout << "***Error in tool re-initialization of src" << endl;
            exit(-1);
        }
        if (dst[i] != ((i + 1) & 0xff))
        {
            cout << "***Error in tool re-initialization of dst" << endl;
            exit(-1);
        }
    }
}

ADDRINT imgStartAdd;
USIZE imgSize;

VOID Instruction(INS ins, VOID* v)
{
    //instrument if ins is app instruction
    if (INS_Address(ins) >= imgStartAdd && INS_Address(ins) < (imgStartAdd + imgSize))
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)MyMemCpy, IARG_PTR, &src, IARG_PTR, &dst, IARG_UINT32, 128, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckCopyAndReset, IARG_PTR, &src, IARG_PTR, &dst, IARG_END);
    }
}

VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        imgStartAdd = IMG_StartAddress(img);
        imgSize     = IMG_SizeMapped(img);
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    for (int i = 0; i < 128; i++)
    {
        src[i] = i;
        dst[i] = (i + 1) & 0xff;
    }

    for (int i = 0; i < 128; i++)
    {
        if (src[i] != i)
        {
            cout << "***Error in tool initialization of src" << endl;
            exit(-1);
        }
        if (dst[i] != ((i + 1) & 0xff))
        {
            cout << "***Error in tool initialization of dst" << endl;
            exit(-1);
        }
    }

    INS_AddInstrumentFunction(Instruction, NULL);

    IMG_AddInstrumentFunction(ImageLoad, NULL);

    // Never returns
    PIN_StartProgram();

    return 1;
}

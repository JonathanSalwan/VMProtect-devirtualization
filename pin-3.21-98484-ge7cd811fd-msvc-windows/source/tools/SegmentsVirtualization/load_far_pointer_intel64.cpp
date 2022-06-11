/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

struct FarPointer16
{
    FarPointer16(unsigned short v0, unsigned short v1) : _farPtr(v0), _segVal(v1), _pad((unsigned)(-1)) {}
    unsigned short _farPtr;
    unsigned short _segVal;
    unsigned int _pad;
};
struct FarPointer32
{
    FarPointer32(unsigned int v0, unsigned int v1) : _farPtr(v0), _segVal(v1), _pad((unsigned)(-1)) {}
    unsigned int _farPtr;
    unsigned int _segVal;
    unsigned int _pad;
};
struct FarPointer64
{
    FarPointer64(unsigned long v0, unsigned long v1) : _farPtr(v0), _segVal(v1), _pad((unsigned long)(-1)) {}
    unsigned long _farPtr;
    unsigned long _segVal;
    unsigned long _pad;
};

extern "C"
{
    unsigned long SetGs16(const FarPointer16* fp, unsigned long upperBits);
    unsigned long SetGs32(const FarPointer32* fp, unsigned long upperBits);
    unsigned long SetGs64(const FarPointer64* fp, unsigned long upperBits);

    unsigned int GetGsVal();

    void MoveMem16ToGs(unsigned int* val32);
    void MoveMem64ToGs(unsigned long* val64);

    void MoveGsToMem16(unsigned int* val32);
    void MoveGsToMem64(unsigned long* val64);
};

const unsigned long upperVal = 0x1122334455667788;
const unsigned long mask16   = 0xffffffffffff0000;
const unsigned long mask32   = 0xffffffff00000000;

int main()
{
    unsigned int newSegVal = 0;
    unsigned long res16    = (upperVal & mask16) | 16;
    FarPointer16 farPtr16(16, newSegVal);
    if (SetGs16(&farPtr16, upperVal) != res16)
    {
        printf("LGS 16-bit failed\n");
        return 0;
    }
    else
    {
        printf("Correct result 0x%lx was loaded to 16-bit register, GS value is 0x%x\n", res16, GetGsVal());
    }

    newSegVal          = 1;
    unsigned int res32 = (unsigned int)-32;
    FarPointer32 farPtr32(res32, newSegVal);
    if (SetGs32(&farPtr32, upperVal) != (unsigned long)res32)
    {
        printf("LGS 32-bit failed, received 0x%lx, expected 0x%lx\n", SetGs32(&farPtr32, upperVal), res32);
        return 0;
    }
    else
    {
        unsigned int gs_val = GetGsVal();
        if (gs_val != newSegVal)
        {
            printf("Incorrect gs value after 32-bit mov 0x%x, expected 0x%x \n", gs_val, newSegVal);
            return 0;
        }
        printf("Correct result 0x%lx was loaded to 32-bit register, GS value is 0x%x\n", res32, gs_val);
    }

    newSegVal           = 2;
    unsigned long res64 = 64;
    FarPointer64 farPtr64(64, newSegVal);
    if (SetGs64(&farPtr64, upperVal) != res64)
    {
        printf("LGS 32-bit failed\n");
        return 0;
    }
    else
    {
        printf("Correct result 0x%lx was loaded to 64-bit register, GS value is 0x%x\n", res64, GetGsVal());
    }

    unsigned int value32 = 0xffff0001;
    MoveMem16ToGs(&value32);
    unsigned int gs_val = GetGsVal();
    res32               = value32 & 0xffff0000;
    MoveGsToMem16(&res32);
    if (value32 != res32)
    {
        printf("Move 16-bit failed, received 0x%x, expected 0x%x\n", res32, value32);
        return 0;
    }
    if (gs_val != 1)
    {
        printf("Incorrect gs value after 16-bit mov\n");
        return 0;
    }
    unsigned long value64 = 0xffffffffffff0002;
    MoveMem64ToGs(&value64);
    gs_val = GetGsVal();
    res64  = 0;
    MoveGsToMem64(&res64);
    if (value64 & 0xffff != res64)
    {
        printf("Move 64-bit failed, received 0x%lx, expected 0x%lx\n", res64, value64 & 0xffff);
        return 0;
    }
    if (gs_val != 2)
    {
        printf("Incorrect gs value after 16-bit mov\n");
        return 0;
    }
    printf("Move instructions 16 and 64-bit mem-to-gs and gs-to-mem were tested successfully\n");

    return 0;
}

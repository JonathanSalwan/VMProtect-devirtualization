/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>
extern "C" int MMXSequence(unsigned int a, unsigned int b, unsigned int c, UINT64* aConcaTb);

int main()
{
    UINT64 aConcaTb;
    UINT32* ptr      = (UINT32*)(&aConcaTb);
    UINT32* ptr1     = ptr + 1;
    unsigned int res = MMXSequence(0xdeadbeef, 0xbaadf00d, 0xfeedf00d, &aConcaTb);
    printf("res is %x  aConcaTb is %x %x\n", res, (*ptr), *(ptr1));
    if (res != 0x3a061f04)
    {
        fprintf(stderr, "***Error unexpected value of res\n");
        return (1);
    }
    if (*ptr != 0xdeadbeef)
    {
        fprintf(stderr, "***Error unexpected value of *ptr\n");
        return (1);
    }
    if (*ptr1 != 0xbaadf00d)
    {
        fprintf(stderr, "***Error unexpected value of *ptr1\n");
        return (1);
    }
    return (0);
}

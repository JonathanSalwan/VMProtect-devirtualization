/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>

extern void FldzFunc();
extern void Fld1Func();
extern void FptagInitFunc();
extern void FldInfFunc();
extern void DoFnstenv(unsigned char* mem28B);
extern void FstpFunc();

/*
 * This program is the application for a test that checks the conversion functions of FPTAG.
 * External functions are in fptag_app_asm.<asm suffix>
 */
int main(int argc, char** argv)
{
    int i;
    int const FpuStackSize = 8;
    unsigned char FnstenvArea[28];
    FptagInitFunc();
    DoFnstenv(FnstenvArea);
    for (i = 0; i < FpuStackSize; i++)
    {
        FldzFunc();
        DoFnstenv(FnstenvArea);
        FstpFunc();
        FldInfFunc();
        DoFnstenv(FnstenvArea);
        FstpFunc();
        Fld1Func();
        DoFnstenv(FnstenvArea);
    }
    return 0;
}

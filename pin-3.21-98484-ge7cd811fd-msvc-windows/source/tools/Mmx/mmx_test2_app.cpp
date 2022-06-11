/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>
extern "C" void FpCompute(double* fPtr);

int main()
{
    double f;
    FpCompute(&f);
    printf("f is %f\n", f);
    return (0);
}

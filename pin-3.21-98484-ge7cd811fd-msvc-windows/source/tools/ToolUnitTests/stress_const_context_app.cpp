/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#if defined(TARGET_WINDOWS)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

unsigned int numCallsToStressTestConstContextAppFunc = 0;
int globVal0;
int globVal1;
int globVal2;
int globVal3;
int globVal4;
int globVal5;
extern "C" DLLEXPORT void StressTestConstContextAppFunc(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5)
{
    numCallsToStressTestConstContextAppFunc++;
    if ((numCallsToStressTestConstContextAppFunc % 100000) == 0)
    {
        printf("%d calls to StressTestConstContextAppFunc\n", numCallsToStressTestConstContextAppFunc);
        fflush(stdout);
    }
    globVal0 = arg0;
    globVal1 = arg1;
    globVal2 = arg2;
    globVal3 = arg3;
    globVal4 = arg4;
    globVal5 = arg5;
}

int main()
{
    for (int i = 0; i < 5000000; i++)
    {
        StressTestConstContextAppFunc(i, i + 1, i + 2, i + 3, i + 4, i + 5);
    }
}

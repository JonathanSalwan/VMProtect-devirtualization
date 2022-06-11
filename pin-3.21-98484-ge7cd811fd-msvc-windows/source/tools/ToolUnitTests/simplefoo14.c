/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This little application is used to test instrumentation of functions replaced by the pin tool.
//
#include <stdio.h>
#include <stdlib.h>

#if defined(TARGET_WINDOWS)
#define EXPORT_SYM __declspec(dllexport)
#else
#define EXPORT_SYM extern
#endif

EXPORT_SYM int FunctionToBeReplaced(int one, int two);

EXPORT_SYM int FunctionCalledByFunctionToBeReplaced(int one, int two);

int x = 0;
int FunctionToBeReplaced(int one, int two)
{
    int res;
    x   = 1;
    res = FunctionCalledByFunctionToBeReplaced(one, two);
    return (res);
}

int FunctionCalledByFunctionToBeReplaced(int one, int two)
{
    x = 2;
    return (one + two);
}

int main()
{
    int res = 0;

    res = FunctionToBeReplaced(6, 8);

    if (res != 1 + 2) // 1 and 2 are the replaced params by the tool (callapp14)
    {
        printf("application got wrong values from FunctionToBeReplaced - expected 3 got %d\n", res);
        exit(-1);
    }

    return 0;
}

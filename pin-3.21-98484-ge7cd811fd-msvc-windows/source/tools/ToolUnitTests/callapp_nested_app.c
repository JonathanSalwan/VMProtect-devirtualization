/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This little application is used to test calling application functions.
//
#include <stdio.h>

#if defined(TARGET_WINDOWS)
#define EXPORT_SYM __declspec(dllexport)
#else
#define EXPORT_SYM extern
#include <stdlib.h>
#endif

EXPORT_SYM long Original1(long param1, long param2);
EXPORT_SYM long Original2(long param1, long param2);

int main()
{
    long res;
    int i;

    for (i = 0; i < 10; i++)
    {
        res = Original1(3, 4);
    }
    if ((unsigned int)res != (unsigned int)9207)
    {
        printf("***ERROR res %ld is unexpected\n", res);
        exit(-1);
    }
    return 0;
}

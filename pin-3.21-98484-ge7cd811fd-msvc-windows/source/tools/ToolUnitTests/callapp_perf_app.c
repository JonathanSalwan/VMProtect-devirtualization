/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This little application is used to test calling application functions.
//
#include <stdio.h>
#include <stdlib.h>

#if defined(TARGET_WINDOWS)
#define EXPORT_SYM __declspec(dllexport)
#else
#define EXPORT_SYM extern
#endif

EXPORT_SYM long Original(long one, long two);
EXPORT_SYM long PreOriginal(long one, long two);

int main()
{
    long res;
    int i;

    PreOriginal(6, 8);
    for (i = 0; i < 500000; i++)
    {
        res = Original(6, 8);
    }
    if (((unsigned int)(res)) != (unsigned int)(1500001))
    {
        printf("***ERROR res %ld is unexpected\n", res);
        exit(-1);
    }
    return 0;
}

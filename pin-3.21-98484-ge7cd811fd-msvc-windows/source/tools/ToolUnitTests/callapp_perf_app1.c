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
#endif

long x = 0;
EXPORT_SYM long Original(long one, long two)
{
    x += (two + one);
    //if (one != 1 || two != 2)
    //{
    //    printf ("got unexpected param value\n");
    //    exit (-1);
    //}
    return (x);
}

EXPORT_SYM long PreOriginal(long one, long two)
{
    x += (two - one);
    //if (one != 1 || two != 2)
    //{
    //    printf ("got unexpected param value\n");
    //    exit (-1);
    //}
    return (x);
}

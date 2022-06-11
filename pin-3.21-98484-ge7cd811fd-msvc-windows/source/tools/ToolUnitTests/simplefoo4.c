/*
 * Copyright (C) 2007-2021 Intel Corporation.
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

EXPORT_SYM void* Bar4(long one, long two);

int main()
{
    void* res;

    res = Bar4(6, 8);

    printf("main: res = %lx\n", (long)res);

    res = Bar4(10, 12);

    printf("main: res = %lx\n", (long)res);

    res = Bar4(14, 16);

    printf("main: res = %lx\n", (long)res);

    return 0;
}

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

EXPORT_SYM int Bar8(int one, int two, int stop);

int main()
{
    int res;

    res = Bar8(6, 8, 0);

    printf("main: res = %d\n", res);

    return 0;
}

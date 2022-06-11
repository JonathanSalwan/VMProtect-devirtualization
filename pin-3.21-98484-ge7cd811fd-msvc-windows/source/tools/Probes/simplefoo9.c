/*
 * Copyright (C) 2008-2021 Intel Corporation.
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

EXPORT_SYM int Bar9(int, int, int, int, int, int, int, int, int);

int main()
{
    int total;

    total = Bar9(1, 2, 3, 4, 5, 6, 7, 8, 9);

    printf("main: total = %d\n", total);

    return 0;
}

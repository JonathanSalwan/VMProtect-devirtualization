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

EXPORT_SYM void Bar10(int, int, int, int, int, int, int, int, int, int);

int main()
{
    Bar10(6, 2, 4, 8, 1, 9, 0, 7, 3, 5);

    return 0;
}

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

EXPORT_SYM short Bar13(short, short, unsigned short, long, long, unsigned long, short, short, unsigned short, long, long,
                       unsigned long);

int main()
{
    Bar13(6, -2, 4, 1111111111, -2222222222, 3333333333, -7, 1, 3, -4444444444, 5555555555, 6666666666);

    return 0;
}

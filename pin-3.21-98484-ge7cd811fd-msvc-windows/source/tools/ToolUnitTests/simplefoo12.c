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

EXPORT_SYM char Bar12(int, int, unsigned int, signed char, signed char, unsigned char, int, int, unsigned int, signed char,
                      signed char, unsigned char);

int main()
{
    Bar12(6, -2, 4, 'z', 't', 'p', -7, 1, 3, 's', 'd', 'f');

    return 0;
}

/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This little application is used to test calling application functions.
// regparms test

#include <stdio.h>

extern int __fastcall FastBar10(int, int, int, int, int, int, int, int, int, int);

int main()
{
    int sum;

    sum = FastBar10(6, 2, 4, 8, 1, 9, 0, 7, 3, 5);

    printf(" main: sum=%d\n", sum);

    return 0;
}

/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This little application is used to test calling application functions.

#include <stdio.h>

extern int __stdcall StdBar10(int, int, int, int, int, int, int, int, int, int);

int main()
{
    int sum;

    sum = StdBar10(1, 2, 3, 4, 5, 6, 7, 8, 9, 0);

    printf(" main: sum=%d\n", sum);

    return 0;
}

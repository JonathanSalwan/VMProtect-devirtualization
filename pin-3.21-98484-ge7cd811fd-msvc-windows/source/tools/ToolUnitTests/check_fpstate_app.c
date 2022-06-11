/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

void trap_me() { printf("In trap me function\n"); }

int main()
{
    /* want to setup the mxcsr exception bits */
    double a, b, c, d;
    unsigned long long cc;

    a = 1;
    b = 3;
    c = 0;

    d = a / b;
    c = 1 / c;

    trap_me();

    cc = *(unsigned long long*)&c;
    printf("d: %.6f c: 0x%llx\n", d, cc);

    return 0;
}

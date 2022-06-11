/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
static int a = 0;

void iprel_imm() { a = 1; }

void iprel_reg(int b) { a = b; }

int reg_iprel() { return a; }

int main()
{
    int c;

    iprel_imm();
    printf("a should be 1; a = %d\n", a);

    iprel_reg(2);
    printf("a should be 2; a = %d\n", a);

    c = reg_iprel();
    printf("c should be 2; c = %d\n", c);
    return 0;
}

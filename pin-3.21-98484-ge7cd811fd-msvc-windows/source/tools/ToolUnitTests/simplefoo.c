/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This little application is used to test calling application functions.
//
#include <stdio.h>

extern Bar();

int main()
{
    unsigned long sp;

    sp = getSP();
    printf("Application stack pointer = 0x%lx\n", sp);

    Bar();

    return 0;
}

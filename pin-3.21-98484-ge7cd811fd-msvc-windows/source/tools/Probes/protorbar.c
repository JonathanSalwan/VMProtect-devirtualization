/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This little application tests recursive calls in probes mode.
//
#include <stdio.h>
static done = 0;

void Bar(int a, int b, int c, int d)
{
    if (done == 0)
    {
        done = 1;
        Bar(a + 20, b + 20, c + 20, d + 20);
    }

    printf("Bar: %d, %d, %d, %d\n", a, b, c, d);
}

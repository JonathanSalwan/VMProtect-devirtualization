/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

extern "C" void one();

// We will use probe on the following function, so its first BBL
// should be long enough to avoid jumps to our trampoline code, even
// when the compiler uses optimizations.
extern "C" void do_nothing()
{
    int n;
    for (int i = 0; i < 100; i++)
    {
        printf(".");
        n++;
    }
    printf("%d\n", n);
}

int main()
{
    do_nothing();
    one();
    printf("Hello, world\n");
    return 0;
}

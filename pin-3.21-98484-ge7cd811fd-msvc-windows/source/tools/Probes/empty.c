/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This little application is used to test replacing a routine
// with an empty function.
//
#include <stdio.h>

extern void Bar(int);

void Foo(int a, int b, int c, int d)
{
    printf("Foo: calling Bar...\n");

    Bar(a);

    printf("Foo: %d, %d, %d, %d\n", a, b, c, d);
}

int main()
{
    Foo(12, 345, 678, 90);
    Foo(11, 22, 33, 44);
    Foo(99, 88, 77, 66);

    return 0;
}

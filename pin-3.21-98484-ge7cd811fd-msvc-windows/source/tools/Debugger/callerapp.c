/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

void Foo(int a, int b, int c);
void Inner(int x);

int main()
{
    printf("This is the main routine\n");
    Foo(1, 2, 3);
    Inner(10);
    return 0;
}

void Foo(int a, int b, int c)
{
    printf("Foo: a=%d, b=%d, c=%d\n", a, b, c);
    Inner(a + b + c);
}

void Inner(int x)
{
    printf("Bar: x=%d\n", x);
    if (x == 10) Inner(12);
}

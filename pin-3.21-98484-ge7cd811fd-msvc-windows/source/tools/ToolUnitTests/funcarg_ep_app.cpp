/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

extern "C" void Foo(int, short, char, int, int, int, int, int, int, int, short, int, char);
extern "C" void Foo1(int, short, char, int, int, int, int, int, int, int, short, int, char);
extern "C" void Foo2(unsigned int, unsigned short, unsigned char, unsigned int, unsigned int, unsigned int, unsigned int,
                     unsigned int, unsigned int, unsigned int, unsigned short, unsigned int, unsigned char);

typedef void (*FN)(int, short, char, int, int, int, int, int, int, int, short, int, char);

int main()
{
    printf("Foo:\n");
    Foo(1, -2, 3, -4, 5, -6, 7, -8, 9, -10, 11, -12, 13);
    printf("\nFoo1:\n");
    Foo1(-1, 2, -3, 4, -5, 6, -7, 8, -9, 10, -11, 12, -13);
    printf("\nFoo2:\n");
    Foo2(-1, 2, -3, 4, -5, 6, -7, 8, -9, 10, -11, 12, -13);
}

void Foo(int x1, short x2, char x3, int x4, int x5, int x6, int x7, int x8, int x9, int x10, short x11, int x12, char x13)
{
    printf("Signed parameters, application results: ; ");
    printf("x1 = %d; ", x1);
    printf("x2 = %d; ", x2);
    printf("x3 = %d; ", x3);
    printf("x4 = %d; ", x4);
    printf("x5 = %d; ", x5);
    printf("x6 = %d; ", x6);
    printf("x7 = %d; ", x7);
    printf("x8 = %d; ", x8);
    printf("x9 = %d; ", x9);
    printf("x10 = %d; ", x10);
    printf("x11 = %d; ", x11);
    printf("x12 = %d; ", x12);
    printf("x13 = %d\n", x13);
}
void Foo1(int x1, short x2, char x3, int x4, int x5, int x6, int x7, int x8, int x9, int x10, short x11, int x12, char x13)
{
    Foo(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13);
}

void Foo2(unsigned int x1, unsigned short x2, unsigned char x3, unsigned int x4, unsigned int x5, unsigned int x6,
          unsigned int x7, unsigned int x8, unsigned int x9, unsigned int x10, unsigned short x11, unsigned int x12,
          unsigned char x13)
{
    printf("Unsigned parameters, application results: ; ");
    printf("x1 = 0x%x; ", x1);
    printf("x2 = 0x%x; ", x2);
    printf("x3 = 0x%x; ", x3);
    printf("x4 = 0x%x; ", x4);
    printf("x5 = 0x%x; ", x5);
    printf("x6 = 0x%x; ", x6);
    printf("x7 = 0x%x; ", x7);
    printf("x8 = 0x%x; ", x8);
    printf("x9 = 0x%x; ", x9);
    printf("x10 = 0x%x; ", x10);
    printf("x11 = 0x%x; ", x11);
    printf("x12 = 0x%x; ", x12);
    printf("x13 = 0x%x\n", x13);
}

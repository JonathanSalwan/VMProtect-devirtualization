/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

void foo() { printf("inside foo\n"); }

void mark1() { printf("inside mark1\n"); }

void mark2() { printf("inside mark2\n"); }

void bar() { printf("inside bar \n"); }

int main()
{
    printf("inside main calling foo\n");
    mark1();
    foo();
    printf("inside main calling bar\n");
    mark2();
    bar();
    return 0;
}

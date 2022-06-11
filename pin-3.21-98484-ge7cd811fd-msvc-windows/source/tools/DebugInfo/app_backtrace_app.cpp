/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

extern "C" void qux()
{
    printf("qux was called\n");
    fflush(stdout);
}

extern "C" void baz() {}

extern "C" void bar()
{
    baz();
    qux();
}

extern "C" void foo()
{
    bar();
    qux();
}

int main()
{
    foo();
    return 0;
}

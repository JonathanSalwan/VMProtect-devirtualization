/*
 * Copyright (C) 2008-2008 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>

extern int foo(int val);

int main()
{
    // Should print "7"
    printf("%d\n", foo(0) + foo(!0));

    return 0;
}

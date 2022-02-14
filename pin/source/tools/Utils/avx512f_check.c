/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

int SupportsAvx512f();

int main()
{
    if (SupportsAvx512f())
        printf("Yes\n");
    else
        printf("No\n");
    return 0;
}

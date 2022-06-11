/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

int SupportsAvx2();

int main()
{
    if (SupportsAvx2())
        printf("Yes\n");
    else
        printf("No\n");
    return 0;
}

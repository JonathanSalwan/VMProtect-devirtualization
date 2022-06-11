/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

int SupportsAmx();

int main()
{
    if (SupportsAmx())
        printf("Yes\n");
    else
        printf("No\n");
    return 0;
}

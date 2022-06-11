/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

int SupportsTsx();

int main()
{
    if (SupportsTsx())
        printf("Yes\n");
    else
        printf("No\n");
    return 0;
}

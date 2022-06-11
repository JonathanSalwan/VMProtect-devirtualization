/*
 * Copyright (C) 2012-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

extern int very_big();

int main()
{
    int num = very_big();
    printf("Result: 0x%x\n", num);
    return 0;
}

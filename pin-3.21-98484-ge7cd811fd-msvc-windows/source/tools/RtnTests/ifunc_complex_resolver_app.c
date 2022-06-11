/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

const char* ifunc();

int main()
{
    printf("Result: %s\n", ifunc());
    return 0;
}

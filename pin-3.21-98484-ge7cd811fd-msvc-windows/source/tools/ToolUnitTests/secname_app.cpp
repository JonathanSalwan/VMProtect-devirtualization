/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#pragma data_seg("dsec")

static char greeting[] = "Hello";

#pragma code_seg("asection")

void report() { printf("%s, world\n", greeting); }

#pragma code_seg(".text")

int main()
{
    report();
    return 0;
}

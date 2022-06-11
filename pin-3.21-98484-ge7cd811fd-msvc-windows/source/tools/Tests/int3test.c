/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* TO COMPILE: 
      gcc -o int3test int3test.c 
 */
#include <stdio.h>
#include <stdint.h>

int main(int argc, char** argv)
{
    int y, i, x = 1;
    if (argc == 2)
        y = atoi(argv[1]);
    else
        y = 5;
    asm volatile("int3");
    for (i = 0; i < 10; i++)
        x = x * y;
    printf("%d\n", x);
}

/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

int main(int argc, char** argv)
{
    int i = 0, x = 0;
    asm volatile("int3"); /*1 */
    asm volatile("int3"); /*2 */
    for (i = 0; i < 10; i++)
        x = x + 1;
    asm volatile("int3"); /*3 */
    return 0;
}

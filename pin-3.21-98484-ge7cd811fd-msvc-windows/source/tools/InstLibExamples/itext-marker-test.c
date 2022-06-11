/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

int main()
{
    int i;
    asm volatile(".byte 0x0f, 0x1f, 0xF3");
    for (i = 0; i < 10; i++)
        ;
    asm volatile(".byte 0x0f, 0x1f, 0xF4");
    return 0;
}

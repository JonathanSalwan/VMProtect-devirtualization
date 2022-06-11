/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

int main()
{
    _asm
        {
        mov ecx, 0x100000
        jcxz foo
        }
    printf("fail");
    return (0);

foo:
    printf("pass");
    return (0);
}

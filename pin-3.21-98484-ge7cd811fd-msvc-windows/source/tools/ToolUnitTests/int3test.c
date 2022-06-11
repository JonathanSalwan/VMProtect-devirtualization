/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

void foo() {}
int main()
{
    foo();
    asm volatile("int3");
    return 0;
}

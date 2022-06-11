/*
 * Copyright (C) 2016-2016 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

void foo(void); // foo is defined in a assembly file func_nops_asm.asm/.s

int main()
{
    foo();
    return 0;
}

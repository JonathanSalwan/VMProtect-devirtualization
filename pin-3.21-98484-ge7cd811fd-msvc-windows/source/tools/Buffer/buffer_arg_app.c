/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

extern int SimpleCmovTest(int i);

int main()
{
    int counter, condition;
    // run 1000 cmov commands
    for (counter = 2000; counter > 1000; counter--)
    {
        condition = SimpleCmovTest(counter);
    }
    // doing this to avoid compiler error - initialized but unused variable
    condition = 0;
    return condition;
}

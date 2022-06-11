/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
extern "C" void relocatable_1();
extern "C" int relocatable_1a();
extern "C" void relocatable_2();
extern "C" void relocatable_3();
extern "C" void non_relocatable_1();
extern "C" void non_relocatable_2();

int main()
{
    relocatable_1();

    int x = relocatable_1a();
    if (x != (int)0xdeadbeef)
    {
        fprintf(stderr, "***Error relocatable_1a returned wrong value %x\n", x);
        return (-1);
    }

    relocatable_2();
    relocatable_3();
    return 0;
}

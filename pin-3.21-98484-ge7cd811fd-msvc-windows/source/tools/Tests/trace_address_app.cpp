/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
//cstdint is supported from cpp11
#include <stdint.h>

int tstfunc1();
int tstfunc2();
int tstfunc3(int k);

int main()
{
    std::cout << std::hex;
    std::cout.setf(std::ios::showbase);
    std::cout.setf(std::ios::uppercase);

    tstfunc1();
    tstfunc2();
    tstfunc3(20);

    std::cout << (uintptr_t)tstfunc1 << "\t";
    std::cout << (uintptr_t)tstfunc2 << "\t";
    std::cout << (uintptr_t)tstfunc3 << "\t";

    return 0;
}

int tstfunc1()
{
    int i = 4;
    return i;
}

int tstfunc2()
{
    if (1 == 0) return 0;
    int j = 0;
    if (j > 0)
    {
        int k = 0;
        j     = k;
    }
    else
    {
        j = 0;
    }
    j += 13;
    return rand() % j;
}

int tstfunc3(int k)
{
    if (k % 2 == 0)
        return rand() % k;
    else
        return tstfunc3(k--);
}

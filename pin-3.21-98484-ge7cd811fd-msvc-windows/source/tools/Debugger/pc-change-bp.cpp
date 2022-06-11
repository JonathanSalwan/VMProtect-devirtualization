/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>

extern "C" int One();
extern "C" int GetValue(int (*)());

int main()
{
    int value = GetValue(One);
    std::cout << "Value is " << std::dec << value << std::endl;
    return 0;
}

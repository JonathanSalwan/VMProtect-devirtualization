/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>

extern "C" __declspec(dllexport) void Foo() { std::cout << "This is the Foo() function" << std::endl; }

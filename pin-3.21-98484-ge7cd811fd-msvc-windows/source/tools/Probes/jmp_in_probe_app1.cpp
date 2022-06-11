/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#if defined(TARGET_WINDOWS)
#include "windows.h"
#define EXPORT_CSYM extern "C" __declspec(dllexport)
#else
#define EXPORT_CSYM extern "C"
#endif
#include <stdio.h>

int xxx = 1;
EXPORT_CSYM int probed_func(int x)
{
    if (x == 0)
    {
        xxx += 2;
        return (1);
    }
    else
    {
        return (0);
    }
    printf("probed_func\n");
}

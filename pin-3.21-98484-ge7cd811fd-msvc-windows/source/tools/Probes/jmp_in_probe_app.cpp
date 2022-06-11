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
EXPORT_CSYM int probed_func_asm(int x);
EXPORT_CSYM int probed_func(int x);

EXPORT_CSYM int main(int argc, char* argv[])
{
    int retVal;
    printf("calling probed func\n");
    fflush(stdout);
    retVal = probed_func(0);
    printf("probed func returned %d\n", retVal);
    fflush(stdout);
    printf("calling probed func\n");
    fflush(stdout);
    retVal = probed_func(1);
    printf("probed func returned %d\n", retVal);
    fflush(stdout);
}

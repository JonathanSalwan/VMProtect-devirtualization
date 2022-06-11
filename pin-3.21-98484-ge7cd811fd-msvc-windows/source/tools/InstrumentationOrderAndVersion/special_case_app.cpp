/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * 
 */
#ifdef TARGET_WINDOWS
#include <windows.h>
#define EXPORT_CSYM extern "C" __declspec(dllexport)
#else
#define EXPORT_CSYM extern "C"
#endif

int a;

EXPORT_CSYM void watch_rtn(int arg) { a = arg; }

EXPORT_CSYM int main(int argc, char* argv[])
{
    watch_rtn(10);
    watch_rtn(20);
    watch_rtn(20);
    watch_rtn(-1);
    watch_rtn(10);
    watch_rtn(10);
    watch_rtn(20);
    return 0;
}

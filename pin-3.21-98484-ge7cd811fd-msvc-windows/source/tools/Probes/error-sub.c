/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//  this application fails so we can check error code.

#include <stdio.h>

#if defined(TARGET_WINDOWS)

#include <windows.h>
#define EXPORT_SYM __declspec(dllexport)

EXPORT_SYM void CheckError()
{
    DWORD lastError = GetLastError();
    fprintf(stdout, "App: error code=%d\n", lastError);
}

#else

#include <errno.h>
#define EXPORT_SYM extern

// We will use probe on the following function, so its first BBL
// should be long enough to avoid jumps to our trampoline code, even
// when the compiler uses optimizations.
EXPORT_SYM void CheckError()
{
    int errnoVal;
    errnoVal = errno;
    fprintf(stdout, "App: error code=%d\n", errnoVal);
    fprintf(stdout, "App: unimportant code=%d\n", errnoVal + errnoVal * errnoVal);
}

#endif

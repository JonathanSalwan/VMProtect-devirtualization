/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

__declspec(dllexport) void SetSystemError(DWORD errorCode) { SetLastError(errorCode); }

__declspec(dllexport) DWORD GetSystemError() { return GetLastError(); }

int main()
{
    DWORD errorCode = 18;

    // exercise heavy use of stack inside test application
    // this specific array size is for regression testing (when run with malloctrace2win)

    char useTheStack[44100];
    useTheStack[0] = 'a';

    SetSystemError(errorCode);
    if (GetSystemError() != errorCode)
    {
        fprintf(stderr, "Error: Bad value returned from GetSystemError\n");
        fflush(stderr);
        exit(1);
    }
    else
    {
        fprintf(stderr, "Success - GetSystemError\n");
    }

    return 0;
}

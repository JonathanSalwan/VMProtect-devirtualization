/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <windows.h>

__declspec(dllexport) int one()
{
    // make the literal 1 be part of the code
    fprintf(stderr, "%d\n", 1);
    fflush(stderr);

    return 1;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) { return 1; }

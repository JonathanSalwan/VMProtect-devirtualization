/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  compile this application without any optimizations (/Od)
 */
#include <Windows.h>
#include <iostream>
#include <math.h>
#include <emmintrin.h>

using std::cout;
using std::endl;
using std::hex;

DWORD WINAPI GetMxcsr(VOID* pParams)
{
    unsigned int i = _mm_getcsr();
    cout << hex << i << endl;
    return 0;
}

int main()
{
    GetMxcsr(0);
    HANDLE threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GetMxcsr, 0, 0, 0);
    WaitForSingleObject(threadHandle, INFINITE);
    CloseHandle(threadHandle);
    return 0;
}

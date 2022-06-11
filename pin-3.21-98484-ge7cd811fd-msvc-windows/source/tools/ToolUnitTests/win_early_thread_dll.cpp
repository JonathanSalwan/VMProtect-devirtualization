/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <Windows.h>
#include <iostream>
#include <stdio.h>

using std::cout;
using std::endl;
using std::flush;
using std::string;
static volatile int i      = 0;
static HANDLE threadHandle = NULL;

extern "C" __declspec(dllexport) __declspec(noinline) DWORD WINAPI ThreadProc(VOID* p)
{
    i++;
    return 0;
}

BOOL WINAPI DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, NULL, 0, NULL);
            if (threadHandle != NULL)
            {
                cout << "Creating thread in DllMain(PROCESS_ATTACH)" << endl << flush;
            }
            else
            {
                cout << "Failed to create thread in DllMain(PROCESS_ATTACH)" << endl << flush;
            }
            i = 12;
            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
        default:
            break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) int Something()
{
    // Proceed only when either the thread was not created or finished.
    WaitForSingleObject(threadHandle, INFINITE);
    return i;
}

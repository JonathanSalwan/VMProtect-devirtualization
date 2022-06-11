/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#define _WIN32_WINNT 0x0400

#include <stdio.h>
#include <windows.h>
#include <iostream>
using std::cerr;
using std::endl;
volatile int doloop = 1;

__declspec(dllexport) int DoLoop() { return doloop; }

VOID CALLBACK My_APCProc(ULONG_PTR dwParam)
{
    if (dwParam == 1)
    {
        return;
    }
    QueueUserAPC(My_APCProc, GetCurrentThread(), (dwParam - 1));
    DWORD status = SleepEx(10, true);
}

DWORD WINAPI ThreadProc1(VOID* p)
{
    while (DoLoop())
    {
        DWORD status = SleepEx(1000, true);
    }
    return 0;
}

DWORD WINAPI ThreadProc2(VOID* p)
{
    while (DoLoop())
    {
        QueueUserAPC(My_APCProc, HANDLE(p), 2);
        Sleep(50);
    }
    return 0;
}

DWORD WINAPI ThreadProc3(VOID* p)
{
    while (DoLoop())
    {
        OutputDebugString("Debugger please help\n");
        Sleep(1);
    }
    return 0;
}

bool CppException()
{
    int h = 0x1234;
    try
    {
        throw 0x1;
    }
    catch (...)
    {
    }
    if (h != 0x1234)
    {
        return false;
    }
    return true;
}

DWORD WINAPI ThreadProc4(VOID* p)
{
    while (DoLoop())
    {
        if (!CppException())
        {
            return -1;
        }
        Sleep(1);
    }
    return 0;
}

void DivideByZero()
{
    static int zero = 0;
    int i           = 1 / zero;
}

DWORD WINAPI ThreadProc5(VOID* p)
{
    while (DoLoop())
    {
        __try
        {
            DivideByZero();
        }
        __except (GetExceptionCode() == EXCEPTION_INT_DIVIDE_BY_ZERO ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            volatile int j = 11;
        }
        Sleep(1);
    }
    return 0;
}

// Number of threads to validate in Pintool
#define NTHREADS 5

int main()
{
    HANDLE threads[NTHREADS];
    threads[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc1, NULL, 0, NULL);
    threads[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc2, (VOID*)(threads[0]), 0, NULL);
    threads[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc3, NULL, 0, NULL);
    threads[3] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc4, NULL, 0, NULL);
    threads[4] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc5, NULL, 0, NULL);

    DWORD ret = WaitForMultipleObjects(NTHREADS, threads, TRUE, 60 * 1000);
    if (ret == WAIT_TIMEOUT)
    {
        cerr << "w_app3 failed!" << endl;
        doloop = 0;
        // Let the threads the opportunity to terminate cleanly
        WaitForMultipleObjects(NTHREADS, threads, TRUE, 10 * 1000);
        exit(-1);
    }
    return 0;
}

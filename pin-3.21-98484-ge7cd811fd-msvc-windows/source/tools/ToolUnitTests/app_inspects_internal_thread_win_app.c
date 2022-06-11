/*
 * Copyright (C) 2014-2014 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This test app examines behavior of OS thread management APIs
 * applied by application to a Pin internal thread.
 * Checked APIs are SuspendThread, GetThreadContext, SetThreadContext, ResumeThread
 * and TerminateThread.
 */

#include <windows.h>

// Exported. Tid of a Pin internal thread is set by Pin tool.
__declspec(dllexport) const unsigned int tid;

int main()
{
    int i;
    for (i = 0; i < 10; i++)
    {
        Sleep(500);
        // tid value is set by Pin tool.
        if (tid != 0)
        {
            CONTEXT context;
            DWORD count;
            BOOL res;
            HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);

            count = SuspendThread(hThread);
            if (count == (DWORD)-1) return 2;

            context.ContextFlags = CONTEXT_FULL;
            if (!GetThreadContext(hThread, &context)) return 3;

            if (!SetThreadContext(hThread, &context)) return 4;

            count = ResumeThread(hThread);
            if (count == (DWORD)-1) return 5;

            // Tries to terminate thread with exit code 7.
            // The call should not succeed.
            if (TerminateThread(hThread, 7)) return 6;

            // Success.
            return 0;
        }
    }
    // tid of Pin internal thread was not set.
    return 1;
}

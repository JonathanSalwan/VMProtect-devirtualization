/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
    This application causes exception in system call LeaveCriticalSection and catches it.
    The exception happens in first bytes of the system call's code
    that will be copied and translated when probed.
*/

#include <windows.h>
#include <stdio.h>

int main(void)
{
    int res = 0;

    __try
    {
        LeaveCriticalSection(NULL);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf("Exception %08X\n", (unsigned long)GetExceptionCode());
        fflush(stdout);
        res++;
    }

    // Try again, if exceptions aren't handled in the library, we'll have a problem

    __try
    {
        LeaveCriticalSection(NULL);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf("Exception %08X\n", (unsigned long)GetExceptionCode());
        fflush(stdout);
        res++;
    }

    return 0;
}

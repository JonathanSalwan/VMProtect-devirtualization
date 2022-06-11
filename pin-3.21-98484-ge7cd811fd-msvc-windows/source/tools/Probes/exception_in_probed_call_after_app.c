/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
  This application causes exception in indirect call instruction and catches it in caller.
  The call instruction is located in code region being replaced by Pin probe.
  Pin translation should not affect propagation of the exception to the exception handler.
*/

#include <windows.h>
#include <stdio.h>

static int(__fastcall* pBar)(int, int, int, int, int, int, int) = 0;

int __fastcall bar(int a0, int a1, int a2, int a3, int a4, int a5, int a6) { return a0 + a1 + a2 + a3 + a4 + a5 + a6; }

__declspec(dllexport) int __fastcall foo(int a0, int a1, int a2, int a3, int a4, int a5, int a6)
{
    // May cause exception due to NULL pointer
    return pBar(a0, a1, a2, a3, a4, a5, a6);
}

int main()
{
    int i;

    __try
    {
        i = foo(1, 2, 3, 4, 5, 6, 7);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // If Pin translated probed code properly, exception will reach the handler
        printf("Exception %08X\n", (unsigned long)GetExceptionCode());
        fflush(stdout);
    }

    pBar = bar;

    __try
    {
        i = foo(1, 2, 3, 4, 5, 6, 7);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // No exception expected
        printf("Exception %08X\n", (unsigned long)GetExceptionCode());
        fflush(stdout);
    }

    printf("result=%d\n", i);
    fflush(stdout);

    return 0;
}

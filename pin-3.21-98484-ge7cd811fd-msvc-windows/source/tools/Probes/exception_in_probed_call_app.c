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

static int (*pBar)() = 0;

int bar() { return 0; }

__declspec(dllexport) int foo()
{
    // May cause exception due to NULL pointer
    return pBar();
}

int main()
{
    int i;

    __try
    {
        i = foo();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // If Pin translated probed code properly, exception will reach the handler
        printf("Exception %08X\n", (unsigned long)GetExceptionCode());
    }

    pBar = bar;

    __try
    {
        i = foo();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // No exception expected
        printf("Exception %08X\n", (unsigned long)GetExceptionCode());
    }

    return i;
}

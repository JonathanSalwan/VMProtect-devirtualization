/*
 * Copyright (C) 2008-2008 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>

__declspec(dllexport) __declspec(naked) int __fastcall foo(int val)
{
    __asm {
        test ecx,ecx
        jz L
        mov eax, 2
        ret
L:
        mov eax, 5
        ret
    }
}

int main()
{
    // Should print "7"
    printf("%d\n", foo(0) + foo(!0));

    return 0;
}

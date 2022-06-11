/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

__declspec(dllexport) void nothing() {}

#if defined(TARGET_IA32)

__declspec(dllexport) __declspec(naked) void* baserel_in_probe()
{
    __asm {
        xor eax, eax
               // Emit base relocation. Located at offset 3 from entry point.
        mov eax, baserel_in_probe
        ret
    }
    ;
}

#endif

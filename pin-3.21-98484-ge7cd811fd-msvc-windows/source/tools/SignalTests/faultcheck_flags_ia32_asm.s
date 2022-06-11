/*
 * Copyright (C) 2008-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text
    .align 4
#if defined(TARGET_MAC)
.globl _SetAppFlagsAndSegv_asm
_SetAppFlagsAndSegv_asm:
#else
.globl SetAppFlagsAndSegv_asm
SetAppFlagsAndSegv_asm:
#endif
    pushf
    pop %eax
    or $0xcd5, %eax
    push %eax
    popf
    mov  $7, %ecx
    mov  %eax, 0(%ecx)
    ret

    .align 4
#if defined(TARGET_MAC)
.globl _ClearAppFlagsAndSegv_asm
_ClearAppFlagsAndSegv_asm:
#else
.globl ClearAppFlagsAndSegv_asm
ClearAppFlagsAndSegv_asm:
#endif
    pushf
    pop %eax
    and $0xfffff000, %eax
    push %eax
    popf
    mov  $7, %ecx
    mov  %eax, 0(%ecx)
    ret
    
    .align 4
#if defined(TARGET_MAC)
.globl _GetFlags_asm
_GetFlags_asm:
#else
.globl GetFlags_asm
GetFlags_asm:
#endif
    pushf
    pop %eax
    ret

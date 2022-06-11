/*
 * Copyright (C) 2010-2013 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text
.align 4

#ifdef TARGET_MAC
.globl _ProcessorSupportsAvx;
_ProcessorSupportsAvx:
#else
.globl ProcessorSupportsAvx;
ProcessorSupportsAvx:
#endif
    push %ebp
    mov  %esp, %ebp
    pusha

    mov $1, %eax

    cpuid

    and  $0x018000000, %ecx
    cmp  $0x018000000, %ecx
    jne .lNOT_SUPPORTED
    mov $0, %ecx

    .balign 1 ; .byte 0x0F
    .balign 1 ; .byte 0x01
    .balign 1 ; .byte 0xD0
    and $6, %eax
    cmp $6, %eax
    jne .lNOT_SUPPORTED
    popa
    mov $1, %eax
    jmp .lDONE3
.lNOT_SUPPORTED:
    popa
    mov $0, %eax
.lDONE3:
    mov %ebp, %esp
    pop %ebp
    ret

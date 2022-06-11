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
    push %rbp
    mov  %rsp, %rbp
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi

    mov $1, %rax

    cpuid

    and $0x018000000, %ecx
    cmp $0x018000000, %ecx
    jne .lNOT_SUPPORTED
    mov $0, %ecx

    # command name
    .byte 0x0F, 0x01, 0xD0

    and $6, %eax
    cmp $6, %eax
    jne .lNOT_SUPPORTED
    mov $1, %rax
    jmp .lDONE3
.lNOT_SUPPORTED:
    mov $0, %rax

.lDONE3:
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx

    mov %rbp, %rsp
    pop %rbp
    ret


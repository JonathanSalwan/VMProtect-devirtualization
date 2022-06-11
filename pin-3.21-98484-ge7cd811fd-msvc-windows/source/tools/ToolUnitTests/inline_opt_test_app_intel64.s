/*
 * Copyright (C) 2010-2014 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.global scratchVals

.text
.global GetIntegerScratches
.type GetIntegerScratches,  @function
GetIntegerScratches:
    push %r12
    lea scratchVals(%rip), %r12
    mov %rax, (%r12)
    mov %rcx, 0x8(%r12)
    mov %rdx, 0x10(%r12)
    mov %rsi, 0x18(%r12)
    mov %rdi, 0x20(%r12)
    mov %r8, 0x28(%r12)
    mov %r9, 0x30(%r12)
    mov %r10, 0x38(%r12)
    mov %r11, 0x40(%r12)
    pop %r12
    ret

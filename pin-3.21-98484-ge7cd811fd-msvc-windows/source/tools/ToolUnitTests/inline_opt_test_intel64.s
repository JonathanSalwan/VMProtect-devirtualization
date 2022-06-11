/*
 * Copyright (C) 2010-2010 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text


.global ZeroOutScratches
.type ZeroOutScratches, @function
ZeroOutScratches:
    xor %rax, %rax
    xor %rcx, %rcx
    xor %rdx, %rdx
    pxor %xmm0, %xmm0
    pxor %xmm1, %xmm1
    pxor %xmm2, %xmm2
    pxor %xmm3, %xmm3
    pxor %xmm4, %xmm4
    pxor %xmm5, %xmm5
    pxor %xmm6, %xmm6
    pxor %xmm7, %xmm7
    pxor %xmm8, %xmm8
    pxor %xmm9, %xmm9
    pxor %xmm10, %xmm10
    pxor %xmm11, %xmm11
    pxor %xmm12, %xmm12
    pxor %xmm13, %xmm13
    pxor %xmm14, %xmm14
    pxor %xmm15, %xmm15
    ret



.global SetIntegerScratchesTo1
.type SetIntegerScratchesTo1,  @function
SetIntegerScratchesTo1:
    xor %rax, %rax
    inc %rax
    xor %rdx, %rdx
    inc %rdx
    xor %rcx, %rcx
    inc %rcx
    xor %rsi, %rsi
    inc %rsi
    xor %rdi, %rdi
    inc %rdi
    xor %r8, %r8
    inc %r8
    xor %r9, %r9
    inc %r9
    xor %r10, %r10
    inc %r10
    xor %r11, %r11
    inc %r11
    ret



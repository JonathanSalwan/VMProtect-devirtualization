/*
 * Copyright (C) 2010-2010 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text


.global ZeroOutScratches
.type ZeroOutScratches, @function
ZeroOutScratches:

    
    xor %eax, %eax
    xor %ecx, %ecx
    xor %edx, %edx
    pxor %xmm0, %xmm0
    pxor %xmm1, %xmm1
    pxor %xmm2, %xmm2
    pxor %xmm3, %xmm3
    pxor %xmm4, %xmm4
    pxor %xmm5, %xmm5
    pxor %xmm6, %xmm6
    pxor %xmm7, %xmm7
    ret



.global SetIntegerScratchesTo1
.type SetIntegerScratchesTo1,  @function
SetIntegerScratchesTo1:
    xor %eax, %eax
    inc %eax
    xor %edx, %edx
    inc %edx
    xor %ecx, %ecx
    inc %ecx
    ret




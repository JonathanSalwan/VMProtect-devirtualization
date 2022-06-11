/*
 * Copyright (C) 2008-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

    .data
    .align 16
xmmval:
    .quad   0x123456789abcdef0
    .quad   0xff00ff000a550a55

    .text
    .align 4
.globl DoXmm
.type DoXmm, @function
DoXmm:
    movl        $1, %eax
    cvtsi2ss    %eax, %xmm0     /* %xmm0 = 1.0 (in lower 32-bits) */
    movl        $2, %eax
    cvtsi2ss    %eax, %xmm1     /* %xmm1 = 2.0 (in lower 32-bits) */
    lea         xmmval, %eax
    movdqa      (%eax), %xmm2
	nop
    ret
.globl ZeroXmms
.type ZeroXmms, @function
ZeroXmms:
    pxor       %xmm0, %xmm0
    pxor       %xmm1, %xmm1
    pxor       %xmm2, %xmm2
	pxor       %xmm3, %xmm3
    ret


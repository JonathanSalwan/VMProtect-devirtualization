/*
 * Copyright (C) 2010-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

    .text
    .align 4
.globl LoadYmm0
.type LoadYmm0, @function
LoadYmm0:
    /*
     * This is "VMOVDQU (%rdi), %ymm0".  We directly specify the machine code,
     * so this test runs even when the compiler doesn't support AVX.
     */
    .byte   0xc5, 0xfe, 0x6f, 0x07

.globl LoadYmm0Breakpoint
LoadYmm0Breakpoint:         /* Debugger puts a breakpoint here */
    ret

/*
 * Copyright (C) 2008-2014 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies Pin's handling of two weird instructions:
 * "call *%rsp" and "jmp *%rsp".
 */

	.text
.globl main
main:
	subq	$8, %rsp

    /*
     * Push a 'ret' instruction, then call it.
     */
    pushq   $0xc3       /* ret */
	call	*%rsp
    popq    %rax

    /*
     * Push a 'jmp *%rcx' instruction, then jump to it.
     */
    lea     .l1(%rip), %rcx
    pushq   $0xe1ff     /* jmp *%rcx */
    jmp     *%rsp
.l1:
    popq    %rax

	xorl	%eax, %eax
	addq	$8, %rsp
	ret

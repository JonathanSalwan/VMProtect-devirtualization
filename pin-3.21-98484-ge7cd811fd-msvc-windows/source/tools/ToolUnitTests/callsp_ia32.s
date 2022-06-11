/*
 * Copyright (C) 2008-2008 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies Pin's handling of two weird instructions:
 * "call *%rsp" and "jmp *%rsp".
 */

	.text
.globl main
main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp

    /*
     * Push a 'ret' instruction, then call it.
     */
    pushl   $0xc3       /* ret */
	call	*%esp
    popl    %eax

    /*
     * Push a 'jmp *%ecx' instruction, then jump to it.
     */
    lea     .l1, %ecx
    pushl   $0xe1ff     /* jmp *%ecx */
    jmp     *%esp
.l1:
    popl    %eax

	xorl	%eax, %eax
	leave
	ret

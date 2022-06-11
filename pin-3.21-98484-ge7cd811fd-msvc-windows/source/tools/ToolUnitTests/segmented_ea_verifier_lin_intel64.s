/*
 * Copyright (C) 2007-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.global TestSegmentedEA
.type TestSegmentedEA, @function


// use of segment register is not an ERROR


TestSegmentedEA:
    push       %rdx
    push       %rcx
    mov        $0x18, %rax
    mov        $8,  %rcx
    mov        %fs:0x10, %rdx
    mov        %rdx, %fs:0x10
	mov        %fs:0(%rax), %rdx
	mov        %rdx, %fs:0(%rax)
	mov        %fs:4(%rax), %rdx
	mov        %rdx, %fs:4(%rax)
	mov        %fs:4(%rax, %rcx, 1), %rdx
	mov        %rdx, %fs:4(%rax, %rcx, 1)
	mov        %fs:0(%rax, %rcx, 1), %rdx
	mov        %rdx, %fs:0(%rax, %rcx, 1)
	mov        %fs:0(, %rcx, 1), %rdx
	mov        %rdx, %fs:0(, %rcx, 1)
	pop        %rcx
	pop        %rdx
    ret




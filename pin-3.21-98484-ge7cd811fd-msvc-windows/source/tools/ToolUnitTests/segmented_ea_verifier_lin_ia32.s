/*
 * Copyright (C) 2007-2007 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.global TestSegmentedEA
.type TestSegmentedEA, @function


// use of segment register is not an ERROR


TestSegmentedEA:
    mov        $0x18, %eax
    mov        $4,  %ecx
    mov        %gs:0x10, %edx
    mov        %edx, %gs:0x10
	mov        %gs:0(%eax), %edx
	mov        %edx, %gs:0(%eax)
	mov        %gs:4(%eax), %edx
	mov        %edx, %gs:4(%eax)
	mov        %gs:4(%eax, %ecx, 1), %edx
	mov        %edx, %gs:4(%eax, %ecx, 1)
	mov        %gs:0(%eax, %ecx, 1), %edx
	mov        %edx, %gs:0(%eax, %ecx, 1)
	mov        %gs:0(, %ecx, 1), %edx
	mov        %edx, %gs:0(, %ecx, 1)
    ret




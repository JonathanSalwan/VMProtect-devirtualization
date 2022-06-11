/*
 * Copyright (C) 2008-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.global SegAccessRtn
.type SegAccessRtn, @function

SegAccessRtn:

push %ebp
mov %esp, %ebp
push %ecx

mov 0x8(%ebp), %eax
mov %eax, %gs:0x10
mov %gs:0x10, %eax

mov $0x10, %ecx
mov %gs:0(%ecx), %eax

movl $100, %gs:0x14
mov $0x10, %ecx
addl %gs:4(%ecx), %eax

pop %ecx
leave
ret

//int SegAccessStrRtn(int x)

.global SegAccessStrRtn
.type SegAccessStrRtn, @function

SegAccessStrRtn:

push %ebp
mov %esp, %ebp

mov 0x8(%ebp), %eax

movl %eax, %gs:0x14
mov $0x14, %esi

lodsl %gs:(%esi), %eax

leave
ret



/*
 * Copyright (C) 2007-2008 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.global MovsTest
.type MovsTest, @function

MovsTest:
push %ebp
mov %esp, %ebp
mov 0x8(%ebp), %esi
mov 0xc(%ebp), %edi
movsl %fs:(%esi), %es:(%edi)
mov %es:-4(%edi), %eax
leave
ret

.global MaskMovqTest
.type MaskMovqTest, @function

MaskMovqTest:
push %ebp
mov %esp, %ebp
mov 0x8(%ebp), %edi    # first operand - an offset under fs
mov 0xc(%ebp), %esi    # second operand - the number to be copied
movl $0xffffffff, %eax
movd %eax, %xmm0       # mask
movd %esi, %xmm1       # the number to be copied
.byte 0x64
maskmovdqu %xmm0, %xmm1
leave
ret

.global PushPopTest
.type PushPopTest, @function

PushPopTest:
push %ebp
mov %esp, %ebp
mov 0x8(%ebp), %edi    # first operand - an offset under fs
mov 0xc(%ebp), %esi    # second operand - the number to be copied
mov %esi, %fs:(%edi)
push %fs:(%edi)
addl $4, %edi
pop %fs:(%edi)
mov %fs:(%edi), %eax
leave
ret

.global CallTest
.type CallTest, @function

CallTest:
push %ebp
mov %esp, %ebp
mov 0x8(%ebp), %edi    # first operand - an offset under fs
call *%fs:(%edi)
leave
ret


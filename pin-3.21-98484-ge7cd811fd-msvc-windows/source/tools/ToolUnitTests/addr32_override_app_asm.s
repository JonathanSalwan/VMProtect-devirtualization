/*
 * Copyright (C) 2012-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.global Do32BitOverride
.type Do32BitOverride, @function
Do32BitOverride:
           mov  (%rsi),    %rax
    addr32 movl  %eax,      (%edi)
           lea   0x8(%rsi), %rsi
    addr32 lea   0x8(%edi), %edi
    addr32 movsl
           lea   0x4(%rsi), %rsi
    addr32 lea   0x4(%edi), %edi 
           push  (%rsi)
    addr32 pop   (%edi)
    addr32 push  (%edi)
    addr32 lea   0xc(%edi), %edi
    addr32 pop   (%edi)
    ret

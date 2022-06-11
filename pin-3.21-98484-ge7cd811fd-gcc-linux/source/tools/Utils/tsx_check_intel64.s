/*
 * Copyright (C) 2012-2014 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text
.align 4

#ifdef TARGET_MAC
.global _SupportsTsx
_SupportsTsx:
#else
.global SupportsTsx
SupportsTsx:
#endif
    push %rbp
    mov  %rsp, %rbp
    .balign 1 ; .byte 0xc7
    .balign 1 ; .byte 0x58
    .balign 1 ; .byte 0x02
    .balign 1 ; .byte 0x00
    .balign 1 ; .byte 0x00
    .balign 1 ; .byte 0x00
    jmp *successLabel(%rip)
abortLabel:
    mov $0, %rax
    jmp *returnLabel(%rip)
successLabel:
    mov $1, %rax
    .balign 1 ; .byte 0x0f
    .balign 1 ; .byte 0x01
    .balign 1 ; .byte 0xd5
returnLabel:
    mov %rbp, %rsp
    pop %rbp
    ret


/*
 * Copyright (C) 2009-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.global SetAppFlags_asm
.type SetAppFlags_asm, @function


SetAppFlags_asm:

    push %ebp
	mov  %esp, %ebp
	mov  8(%ebp), %eax
    pushf
    pop %ecx
    or %eax, %ecx
    mov %eax, %edx
    push %ecx
    popf
    leave
    ret


.global ClearAcFlag_asm
.type ClearAcFlag_asm, @function
ClearAcFlag_asm: 

    
    pushf
    pop %ecx
    and $0xfffbffff, %ecx
    push %ecx
    popf
    ret

    .align 4
.globl GetFlags_asm
.type GetFlags_asm, @function
GetFlags_asm:
    pushf
    pop %eax
    ret

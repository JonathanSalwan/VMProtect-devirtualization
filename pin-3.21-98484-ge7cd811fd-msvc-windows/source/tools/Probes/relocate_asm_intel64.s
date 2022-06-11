/*
 * Copyright (C) 2009-2018 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>
nothing:
    ret

set_dead_beef:
    mov $0xdeadbeef, %rax
    ret

# A function with call in the first 5 bytes
DECLARE_FUNCTION(relocatable_1)
.global NAME(relocatable_1)
NAME(relocatable_1):
    call nothing
    push %rbp
    mov %rsp, %rbp
    call nothing
    call nothing
    leave
    ret
END_FUNCTION(relocatable_1)

#  A function with a call in the first 5 bytes, that does an indirect call to another function
DECLARE_FUNCTION(relocatable_1a)
.global NAME(relocatable_1a)
NAME(relocatable_1a):
    call nothing
    push %rbp
    mov %rsp, %rbp
    call nothing
    call nothing
    mov $1, %rax
    lea set_dead_beef(%rip), %rcx
    callq *%rcx
    leave
    ret
END_FUNCTION(relocatable_1a)




# A function with short first bb
DECLARE_FUNCTION(relocatable_2)
.global NAME(relocatable_2)
NAME(relocatable_2):
    xor %rax, %rax
    L:
    call nothing
    cmpq $1, %rax
    je LBB0
    inc %rax
    jmp L    
 LBB0:
    ret
    xor %rax, %rax
    xor %rbx, %rbx
END_FUNCTION(relocatable_2)

# A function with short first bb
DECLARE_FUNCTION(relocatable_3)
.global NAME(relocatable_3)
NAME(relocatable_3):
    xor %rax, %rax
    LBB1:
    mov 2(%rip), %rax
    call nothing
    cmpq $1, %rax
    jne LBB2
    movq $0, %rax
    jmp LBB1    
 LBB2:
    ret
    xor %eax, %eax
    xor %ebx, %ebx
END_FUNCTION(relocatable_3)

# A function with indirect jump
DECLARE_FUNCTION(non_relocatable_1)
.global NAME(non_relocatable_1)
NAME(non_relocatable_1):
    push %rbp
NR1L:
    mov %rsp, %rbp
    mov %rdi, %rax
    call nothing
    call nothing
    call nothing
    jmp *%rax
    je NR1L
    leave
    ret
END_FUNCTION(non_relocatable_1)

# A function with fallthru at the end
DECLARE_FUNCTION(non_relocatable_2)
.global NAME(non_relocatable_2)
NAME(non_relocatable_2):
    push %rbp
NR2M:
    mov %rsp, %rbp
    test %rax, %rax
    jb NR2M
    je NR2L
    leave
    ret
NR2L:
    inc %rax
    test %rax, %rax
    je NAME(non_relocatable_2)
END_FUNCTION(non_relocatable_2)


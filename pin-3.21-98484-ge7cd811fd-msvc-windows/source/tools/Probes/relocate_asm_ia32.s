/*
 * Copyright (C) 2009-2018 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

nothing:
    ret

set_dead_beef:
    mov $0xdeadbeef, %eax
    ret

# A function with jmp in the first 5 bytes
DECLARE_FUNCTION(relocatable_1)
.global NAME(relocatable_1)
NAME(relocatable_1):
    push %ebp
    mov %esp, %ebp
LBB0:
    call nothing
    call nothing
    movl $1, %eax
    test %eax, %eax
    je LBB0
    leave
    ret
END_FUNCTION(relocatable_1)


# A function with jmp in the first 5 bytes, that does an indirect call to another function
DECLARE_FUNCTION(relocatable_1a)
.global NAME(relocatable_1a)
NAME(relocatable_1a):
    push %ebp
    mov %esp, %ebp
LBB1:
    mov $1, %eax
    lea set_dead_beef, %ecx
    calll *%ecx
    test %eax, %eax
    je LBB1
    leave
    ret
END_FUNCTION(relocatable_1a)



# A function with short first bb
DECLARE_FUNCTION(relocatable_2)
.global NAME(relocatable_2)
NAME(relocatable_2):
    xor %eax, %eax
    LBB2:
    call nothing
    cmpl $1, %eax
    je LBB3
    inc %eax
    jmp LBB2    
 LBB3:
    ret
    xor %eax, %eax
    xor %ebx, %ebx
END_FUNCTION(relocatable_2)

# A function with short first bb
DECLARE_FUNCTION(relocatable_3)
.global NAME(relocatable_3)
NAME(relocatable_3):
    xor %eax, %eax
    LBB4:
    mov $2, %eax
    call nothing
    cmpl $1, %eax
    jne LBB5
    mov $0, %eax
    jmp LBB4    
 LBB5:
    ret
    xor %eax, %eax
    xor %ebx, %ebx
END_FUNCTION(relocatable_3)

# A function with indirect jump
DECLARE_FUNCTION(non_relocatable_1)
.global NAME(non_relocatable_1)
NON_RELOCATABLE_1_START:
NAME(non_relocatable_1):
    push %ebp
LBB6:
    mov %esp, %ebp
    mov 8(%ebp), %eax
    call nothing
    call nothing
    call nothing
    je LBB6
    jmpl *%eax
    leave
    ret
END_FUNCTION(non_relocatable_1)

# A function with fallthru at the end
DECLARE_FUNCTION(non_relocatable_2)
.global NAME(non_relocatable_2)
NAME(non_relocatable_2):
    push %ebp
LBB8:
    mov %esp, %ebp
    test %eax, %eax
    jb LBB8
    je LBB7
    leave
    ret
LBB7:
    inc %eax
    test %eax, %eax
    je NAME(non_relocatable_2)
END_FUNCTION(non_relocatable_2)


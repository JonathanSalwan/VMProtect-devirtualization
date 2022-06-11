/*
 * Copyright (C) 2007-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

# 
# contains important code patterns
# doesn't actually do anything

# make sure the relocated entry will copy more than one basic block.
.text
.global NAME(do_nothing)
DECLARE_FUNCTION(do_nothing)

NAME(do_nothing):
    test %rax, %rax
    je lab1

    xor %rax, %rax
    xor %rax, %rax

lab1:
    xor %rax, %rax
    xor %rax, %rax

    ret

#make sure the relocated entry will correctly handle a jump as the last
#instruction in the basic block.
.global NAME(nothing_doing)
DECLARE_FUNCTION(nothing_doing)

NAME(nothing_doing):
    xor %rax, %rax
    test %rax, %rax
    je lab1

    xor %rax, %rax
    xor %rax, %rax

lab2:
    xor %rax, %rax
    xor %rax, %rax

    ret


# call should be replaced with a push/jmp when relocated.
#
.global NAME(call_function)
DECLARE_FUNCTION(call_function)

NAME(call_function):
    push %rax
    push %rbx
    call NAME(do_nothing)
    pop %rbx
    pop %rax
    ret


# make sure this code pattern works
#
.global NAME(call_nothing)
DECLARE_FUNCTION(call_nothing)

NAME(call_nothing):
    push %rax
    mov %rax, %rax
    push %rbx
    call NAME(do_nothing)
    pop %rbx
    pop %rax
    ret

#make sure this code pattern works
#
.global NAME(shorty)
DECLARE_FUNCTION(shorty)
	
NAME(shorty):	
	xor %rax, %rax
	jmp lab3
	nop
	nop
	nop
lab3:	
	nop
	nop
	nop
	ret
			

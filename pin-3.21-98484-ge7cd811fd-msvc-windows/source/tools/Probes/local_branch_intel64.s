/*
 * Copyright (C) 2007-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

# this routine should generate an error because the target of a local branch
# is within the probe space.
#
.text
.global NAME(do_nothing)
DECLARE_FUNCTION(do_nothing)

NAME(do_nothing):
    mov %rax, 0x0
LBB1:
    xor %rax, %rax
    test %rax, %rax
    je LBB2

    xor %rax, %rax
    xor %rax, %rax
    jmp LBB1

LBB2:
    xor %rax, %rax
    xor %rax, %rax

    ret


# This code pattern will result in an error because the call 
# is less than the size of the probe, and when relocated, the 
# push of the original IP will cause a branch into the probe.
#
.global NAME(call_function)
DECLARE_FUNCTION(call_function)

NAME(call_function):
    call NAME(do_nothing)
    push %rbx
    pop %rbx
    ret



# Test an unconditional jump in the middle of the probe area.
#	
.global NAME(good_jump)
DECLARE_FUNCTION(good_jump)

NAME(good_jump):
    xchg %rax, %rax
    jmp LBB3

    push %rbx
    pop %rbx

LBB3:
    xor %rax, %rax
    xor %rax, %rax

    ret


# This code can be relocated together with the
# mov $ip instruction.
#	
.global NAME(move_ip)
DECLARE_FUNCTION(move_ip)

MOVE_IP_START:
NAME(move_ip):
    xchg %rax, %rax
    jmp LBB4

    push %rbx
    mov %rbx, 0x1b(%rip)	
    pop %rbx
LBB4:
    movq $1, %rax
    test %rax, %rax
    je LBB4
    xor %rax, %rax
    xor %rax, %rax

    ret

MOVE_IP_END:
#ifndef TARGET_MAC
.size mov_ip, MOVE_IP_END - MOVE_IP_START
#endif	

# This code should not be able to be relocated because of the
# conditional jump as the last instruction.
#	
.global NAME(fall_thru)
DECLARE_FUNCTION(fall_thru)

NAME(fall_thru):
    xchg %rax, %rax
    jmp LBB5

    push %rbx
    pop %rbx

LBB5:
    xor %rax, %rax
    xor %rax, %rax
    mov %rax, 0x5
    cmp %rax, 0	
    jz LBB5 
    nop
    nop
    nop
	



# This code should not be able to be relocated because of the
# jump outside of the function.
#	
.global NAME(bad_jump)
DECLARE_FUNCTION(bad_jump)

NAME(bad_jump):
    xchg %rax, %rax
    jmp LBB6

    push %rbx
    pop %rbx

LBB6:
    jmp NAME(do_nothing)
    xor %rax, %rax
    xor %rax, %rax
    ret
	


# This code should not be able to be relocated because of the
# call function.
#	
.global NAME(bad_call)
DECLARE_FUNCTION(bad_call)

NAME(bad_call):
    xchg %rax, %rax
    jmp LBB7

    push %rbx
    pop %rbx

LBB7:
    call NAME(do_nothing)
    xor %rax, %rax
    xor %rax, %rax
    ret
	


	
# This code should not be able to be relocated because of the
# target after the last instruction.
#	
.global NAME(high_target)
DECLARE_FUNCTION(high_target)

NAME(high_target):
    xchg %rax, %rax
    jmp LBB8

    push %rbx
    pop %rbx

    xor %rax, %rax
    xor %rax, %rax
    mov %rax, 0x5
    cmp %rax, 0	
    ret
	
LBB8:	
    nop
    nop
    nop
	
	

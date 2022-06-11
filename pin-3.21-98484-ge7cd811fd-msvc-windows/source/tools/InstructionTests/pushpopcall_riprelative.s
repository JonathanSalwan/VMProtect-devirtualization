/*
 * Copyright (C) 2014-2016 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  A small test which doesn't link in a C runtime.
 *  It provides a  menagerie of different unpleasant push instructions, and
 *  other instructions which exercise unpleasant corners of the instruction set,
 *  such as access to (virtualised) segment registers.
 */
        .data
inputval:  .long  0x01234567
outputval: .long  0x00000000

        .text
.globl _start
_start:

        # Test 1 push rip relative indirect value to the stack
        mov     $1, %rdi       # %rdi holds the test number, returned as status of exit on failure
        pushq   inputval(%rip)
        movq    inputval(%rip), %rax
        cmp     (%rsp),%rax
        jne     1f

        # Test 2 pop rip relative indirect value from the stack
        incq    %rdi
        popq    outputval(%rip) # Value should be the same as the value previously pushed.
        cmp     outputval(%rip),%rax
        jne     1f

        # Test 3 call a routine through rip.
        incq    %rdi   
        mov     $1,%rax
        lea     stubRoutine(%rip),%r13
        mov     %r13,outputval(%rip)
        call    *outputval(%rip)
        cmp     $0,%rax
        jne     1f

        # Exit
	movq	$0,%rdi		# first argument: exit code
1:  
	movq	$231,%rax	# system call number (sys_exit)
	syscall 		# call kernel

        # Paranoia in case we don't exit
        # Force a SEGV
        movq    $0,0

# Null routine
stubRoutine:
        mov     $0,%rax
        ret

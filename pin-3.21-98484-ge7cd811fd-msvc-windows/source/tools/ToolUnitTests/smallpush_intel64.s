/*
 * Copyright (C) 2009-2016 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  A small test which doesn't link in a C runtime.
 *  It provides a  menagerie of different unpleasant push instructions, and
 *  other instructions which exercise unpleasant corners of the instruction set,
 *  such as access to (virtualised) segment registers.
 */
        .data
value:  .long  0x01234567

        .text
.globl _start
_start:
        mov     $1, %rdi       # %rdi holds the test number, returned as status of exit on failure
	pushq   $0
        cmpq    $0,(%rsp)
        jne     1f

        # Test 2
        incq    %rdi
	pushq   $-1
        cmpq    $-1,(%rsp)
        jne     1f

        # Test 3
        incq    %rdi
        mov     %rsp,%rcx
	pushw   $-1
        cmpw    $-1,(%rsp)
        jne     1f
        # Test 4
        incq    %rdi
        lea     2(%rsp),%rax   # check the stack moved down by two
        cmp     %rax,%rcx
        jne     1f
        lea     -2(%rsp),%rsp # align the stack again
        
        # Test 5
        incq    %rdi   
#ifdef __PIC__
        pushq   value(%rip)
        movq    value(%rip), %rax
#else
        pushq   value
        movq    value, %rax
#endif
        cmp     (%rsp),%rax
        jne     1f

        # Test 6
        incq    %rdi
        pushq   (%rsp)
        cmp     %rax, (%rsp)
        jne     1f

        # Test 7
        incq    %rdi
        decq    8(%rsp)
        dec     %rax
        pushq    8(%rsp)
        cmp     %rax,(%rsp)
        jne     1f

        # Some tests on virtualized segment registers.
        # Test 8
        incq    %rdi
        mov     %rsp,%rcx       
        push    %gs
        lea     8(%rsp),%rax   # check the stack moved down by eight
        cmpq    %rcx,%rax
        jne     1f

        # Test 9, check the segment register was store correctly
        incq    %rdi
        mov     %gs,%rax
        cmp     %rax,(%rsp)
        jne     1f

        # Test 10
        # Pop the segment register
        incq    %rdi
        mov     %rsp,%rcx       
        pop     %gs
        lea     -8(%rsp),%rax   # check the stack moved up by four
        cmpq    %rcx,%rax
        jne     1f

        # Tests on pushf/popf
        # Test 11
        incq    %rdi
        mov     %rsp,%rcx       
        pushfw
        lea     2(%rsp),%rax   # check the stack moved down by 2
        cmpq    %rcx,%rax
        jne     1f
        lea     2(%rsp),%rsp   # align the stack again

        # Test 12
        incq    %rdi
        mov     %rsp,%rcx       
        pushfq
        lea     8(%rsp),%rax   # check the stack moved down by 8
        cmpq    %rcx,%rax
        jne     1f

        # Test 13, popf
        incq    %rdi
        movq    $0x282,(%rsp)
        mov     %rsp,%rcx       
        popfq
        lea     -8(%rsp),%rax   # check the stack moved up by 8
        cmpq    %rcx,%rax
        jne     1f

        # Test 14
        incq    %rdi    
        mov     %rsp,%rcx       
        push    %sp
        lea     2(%rsp),%rax   # check the stack moved down by 2
        cmpq    %rcx,%rax
        jne     1f

        # Test 15, check the value pushed
        incq    %rdi
        cmpw    (%rsp), %ax
        jne     1f
        
        # Test 16
        incq    %rdi    
        mov     %rsp,%rcx       
        pop     %sp
        lea     -2(%rsp),%rax   # check the stack moved up by 2
        cmpq    %rcx,%rax
        jne     1f

        # Test 17 Enter
        incq    %rdi
        lea     -8(%rsp), %rcx
        mov     $0x12345678,%rbp
        enter   $20,$0
        cmp     %rcx,%rbp       # check %rbp updated correctly
        jne     1f
        # Test 18
        incq    %rdi
        lea     -20(%rcx),%rcx  # check %rsp updated correctly
        cmp     %rcx,%rsp
        jne     1f
        # Test 19
        incq    %rdi
        cmpq    $0x12345678,20(%rsp) # check old %rbp saved correctly
        jne     1f
        
        # Test 20, not a push, rather a segment register store
        incq    %rdi
        mov     %gs, (%rsp)
        mov     %gs, %cx
        cmpw    %cx, (%rsp)
        jne     1f

        # Test 21, mov from gs
        incq    %rdi
        mov     %gs, (%rsp)
        mov     %gs, %rax
        cmp     (%rsp), %rax
        jne     1f

        # Test 22, mov to gs
        incq    %rdi
        mov     (%rsp),%gs
        # no verification here, just tests code generation.
        
        # Test 23, pop (%rsp)
        incq    %rdi
        movq    $0, (%rsp)
        movq    $1,8(%rsp)
        popq    (%rsp)
        cmpq    $0, (%rsp)
        jne     1f

        # Test 24, pop 8(%rsp)
        incq    %rdi
        movq    $1,16(%rsp)
        popq    8(%rsp)
        cmpq    $0,8(%rsp)
        jne     1f

        # Test 25, popw (%rsp)
        incq    %rdi
        movw    $0,(%rsp)
        movw    $1,2(%rsp)
        popw    (%rsp)
        cmpw    $0,(%rsp)
        jne     1f

        # Align the stack again.
        lea     6(%rsp), %rsp

        # Test 26, translation of call 0
        inc     %rdi
        xor     %rax,%rax
        je      2f
        call    0
2:

        # Test 27 call indirect through the stack
        inc     %rdi
        mov     $1,%rax
#ifdef __PIC__
        lea     stubRoutine(%rip),%rax 
        movq    %rax,8(%rsp)
#else
        movq    $stubRoutine,8(%rsp)
#endif
        call    *8(%rsp)
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

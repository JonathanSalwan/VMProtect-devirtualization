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
value:       .long  0x01234567

        .text
.globl _start
_start:
        mov     $1, %ebx       # %ebx holds the test number, returned as status of exit on failure
	pushl   $0
        cmpl    $0,(%esp)
        jne     1f

        # Test 2
        incl    %ebx
	pushl   $-1
        cmpl    $-1,(%esp)
        jne     1f

        # Test 3
        incl    %ebx
        mov     %esp,%ecx
	pushw   $-1
        cmpw    $-1,(%esp)
        jne     1f
        # Test 4
        incl    %ebx
        lea     2(%esp),%eax   # check the stack moved down by two
        cmp     %eax,%ecx
        jne     1f
        lea     -2(%esp),%esp # align the stack again
        
        # Test 5
        incl    %ebx   
        pushl   value
        movl    value, %eax
        cmp     (%esp),%eax
        jne     1f

        # Test 6
        incl    %ebx
        pushl   (%esp)
        cmp     %eax, (%esp)
        jne     1f

        # Test 7
        incl    %ebx
        decl    4(%esp)
        dec     %eax
        pushl    4(%esp)
        cmp     %eax,(%esp)
        jne     1f

        # Test 8
        incl    %ebx
        mov     %esp,%ecx       
        push    %cs
        lea     4(%esp),%eax   # check the stack moved down by four
        cmpl    %ecx,%eax
        jne     1f
        # Test 9 
        incl    %ebx
        mov     %cs,%eax
        cmp     %eax,(%esp)
        jne     1f

        # Some tests on virtualized segment registers.
        # Test 10
        incl    %ebx
        mov     %esp,%ecx       
        push    %gs
        lea     4(%esp),%eax   # check the stack moved down by four
        cmpl    %ecx,%eax
        jne     1f

        # Test 11, check the segment register was store correctly
        incl    %ebx
        mov     %gs,%eax
        cmp     %eax,(%esp)
        jne     1f

        # Test 12
        # Pop the segment register
        incl    %ebx
        mov     %esp,%ecx       
        pop     %gs
        lea     -4(%esp),%eax   # check the stack moved up by four
        cmpl    %ecx,%eax
        jne     1f

        # Tests on pushf/popf
        # Test 13
        incl    %ebx
        mov     %esp,%ecx       
        pushfw
        lea     2(%esp),%eax   # check the stack moved down by 2
        cmpl    %ecx,%eax
        jne     1f
        lea     2(%esp),%esp   # align the stack again

        # Test 14
        incl    %ebx
        mov     %esp,%ecx       
        pushfl
        lea     4(%esp),%eax   # check the stack moved down by 4
        cmpl    %ecx,%eax
        jne     1f

        # Test 15, popf
        incl    %ebx
        movl    $0x282,(%esp)
        mov     %esp,%ecx       
        popfl
        lea     -4(%esp),%eax   # check the stack moved up by 4
        cmpl    %ecx,%eax
        jne     1f

        # Test 16
        incl    %ebx    
        mov     %esp,%ecx       
        push    %sp
        lea     2(%esp),%eax   # check the stack moved down by 2
        cmpl    %ecx,%eax
        jne     1f

        # Test 17, check the value pushed
        incl    %ebx
        cmpw    (%esp), %ax
        jne     1f
        
        # Test 18
        incl    %ebx    
        mov     %esp,%ecx       
        pop     %sp
        lea     -2(%esp),%eax   # check the stack moved up by 2
        cmpl    %ecx,%eax
        jne     1f

        # Test 19 Enter
        incl    %ebx
        lea     -4(%esp), %ecx
        mov     $0x12345678,%ebp
        enter   $20,$0
        cmp     %ecx,%ebp       # check %ebp updated correctly
        jne     1f
        # Test 20
        incl    %ebx
        lea     -20(%ecx),%ecx  # check %esp updated correctly
        cmp     %ecx,%esp
        jne     1f
        # Test 21
        incl    %ebx
        cmpl    $0x12345678,20(%esp) # check old %ebp saved correctly
        jne     1f
        
        # Test 22, not a push, rather a segment register store
        incl    %ebx
        mov     %gs, (%esp)
        mov     %gs, %cx
        cmpw    %cx, (%esp)
        jne     1f

        # Test 23 pusha
        incl    %ebx
        mov     %esp,%ecx
        mov     $1,%edi
        mov     $2,%esi
        pushal
        xor     %edi,%edi
        xor     %esi,%esi
        lea     -4*8(%ecx),%ecx
        cmp     %ecx,%esp
        jne     1f

        # Test 24 popa
        incl    %ebx
        popal                   # Will pop %ecx, but that's fine.
        cmp     %ecx,%esp
        jne     1f

        # Test 25, Compare the other two registers we set
        incl    %ebx
        cmp     $1, %edi
        jne     1f
        cmp     $2,%esi
        jne     1f
        
        # Test 26, mov from gs
        incl    %ebx
        mov     %gs, (%esp)
        mov     %gs, %eax
        cmp     (%esp), %eax
        jne     1f

        # Test 27, mov to gs
        incl    %ebx
        mov     (%esp),%gs
        # no verification here, just tests code generation.

        # Test 28, pop (%esp)
        incl    %ebx
        movl    $0, (%esp)
        movl    $1,4(%esp)
        popl    (%esp)
        cmpl    $0, (%esp)
        jne     1f

        # Test 29, pop 4(%esp)
        incl    %ebx
        movl    $1,8(%esp)
        popl    4(%esp)
        cmpl    $0,4(%esp)
        jne     1f

        # Test 30, popw (%esp)
        incl    %ebx
        movw    $0,(%esp)
        movw    $1,2(%esp)
        popw    (%esp)
        cmpw    $0,(%esp)
        jne     1f

        # Test 31, translation of call 0
        incl    %ebx
        xor     %eax,%eax
        je      2f
        call    0
2:
        # Test 32 call indirect through the stack
        incl    %ebx
        mov     $1,%eax
        movl    $stubRoutine,4(%esp)
        call    *4(%esp)
        cmp     $0,%eax
        jne     1f
        
# Exit
	movl	$0,%ebx		# first argument: exit code
1:  
	movl	$1,%eax		# system call number (sys_exit)
	int	$0x80		# call kernel


# Null routine
stubRoutine:
        mov     $0,%eax
        ret

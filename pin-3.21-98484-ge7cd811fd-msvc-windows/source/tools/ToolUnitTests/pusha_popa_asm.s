/*
 * Copyright (C) 2007-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

	.text
.globl DoPushA16
#ifndef TARGET_MAC
	.type	DoPushA16,@function
#endif
DoPushA16:
	pushl	%ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi

    movl    28(%esp), %eax          # third parameter (&inSp)
    mov     %sp, 0(%eax)            # save SP before pusha

    mov     20(%esp), %edi          # first parameter (&inRegs)
    mov     0(%edi), %ax            # ax = inRegs->ax
    mov     2(%edi), %cx            # cx = inRegs->cx
    mov     4(%edi), %dx            # dx = inRegs->dx
    mov     6(%edi), %bx            # bx = inRegs->bx
                                    # (skip sp value)
    mov     10(%edi), %bp           # bp = inRegs->bp
    mov     12(%edi), %si           # si = inRegs->si
    mov     14(%edi), %di           # di = inRegs->di

    // This is the encoding for `pushaw`. The `pushaw` mnemonic is not known
    // on old Clang versions, so we encode the instruction as raw bytes.
    //
    .byte 0x66,0x60

    mov     (16+24)(%esp), %edi     # second parameter (&outRegs)
    mov     14(%esp), %ax           # outRegs->ax = push'd ax value
    mov     %ax, 0(%edi)
    mov     12(%esp), %ax           # outRegs->cx = push'd cx value
    mov     %ax, 2(%edi)
    mov     10(%esp), %ax           # outRegs->dx = push'd dx value
    mov     %ax, 4(%edi)
    mov     8(%esp), %ax            # outRegs->bx = push'd bx value
    mov     %ax, 6(%edi)
    mov     6(%esp), %ax            # outRegs->sp = push'd sp value
    mov     %ax, 8(%edi)
    mov     4(%esp), %ax            # outRegs->bp = push'd bp value
    mov     %ax, 10(%edi)
    mov     2(%esp), %ax            # outRegs->si = push'd si value
    mov     %ax, 12(%edi)
    mov     0(%esp), %ax            # outRegs->di = push'd di value
    mov     %ax, 14(%edi)

    lea     16(%esp), %esp

    popl    %esi
    popl    %edi
    popl    %ebx
    popl    %ebp
	ret


	.text
.globl DoPushA32
#ifndef TARGET_MAC
	.type	DoPushA32,@function
#endif
DoPushA32:
	pushl	%ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi

    movl    28(%esp), %eax          # third parameter (&inSp)
    mov     %esp, 0(%eax)           # save SP before pusha

    mov     20(%esp), %edi          # first parameter (&inRegs)
    mov     0(%edi), %eax           # eax = inRegs->eax
    mov     4(%edi), %ecx           # ecx = inRegs->ecx
    mov     8(%edi), %edx           # edx = inRegs->edx
    mov     12(%edi), %ebx          # ebx = inRegs->ebx
                                    # (skip esp value)
    mov     20(%edi), %ebp          # ebp = inRegs->ebp
    mov     24(%edi), %esi          # esi = inRegs->esi
    mov     28(%edi), %edi          # edi = inRegs->edi

    pusha

    mov     (32+24)(%esp), %edi     # second parameter (&outRegs)
    mov     28(%esp), %eax          # outRegs->eax = push'd eax value
    mov     %eax, 0(%edi)
    mov     24(%esp), %eax          # outRegs->ecx = push'd ecx value
    mov     %eax, 4(%edi)
    mov     20(%esp), %eax          # outRegs->edx = push'd edx value
    mov     %eax, 8(%edi)
    mov     16(%esp), %eax          # outRegs->ebx = push'd ebx value
    mov     %eax, 12(%edi)
    mov     12(%esp), %eax          # outRegs->esp = push'd esp value
    mov     %eax, 16(%edi)
    mov     8(%esp), %eax           # outRegs->ebp = push'd ebp value
    mov     %eax, 20(%edi)
    mov     4(%esp), %eax           # outRegs->esi = push'd esi value
    mov     %eax, 24(%edi)
    mov     0(%esp), %eax           # outRegs->edi = push'd edi value
    mov     %eax, 28(%edi)

    lea     32(%esp), %esp

    popl    %esi
    popl    %edi
    popl    %ebx
    popl    %ebp
	ret


.globl DoPopA16
#ifndef TARGET_MAC
	.type	DoPopA16,@function
#endif
DoPopA16:
	pushl	%ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi

    lea     -16(%esp), %esp         # reserve stack space for register block that will be pop'd

    movl    (16+20)(%esp), %eax     # first parameter (&inRegs)
    mov     0(%eax), %bx            # copy inRegs->ax to stack
    mov     %bx, 14(%esp)
    mov     2(%eax), %bx            # copy inRegs->cx to stack
    mov     %bx, 12(%esp)
    mov     4(%eax), %bx            # copy inRegs->dx to stack
    mov     %bx, 10(%esp)
    mov     6(%eax), %bx            # copy inRegs->bx to stack
    mov     %bx, 8(%esp)
    mov     8(%eax), %bx            # copy inRegs->sp to stack
    mov     %bx, 6(%esp)
    mov     10(%eax), %bx           # copy inRegs->bp to stack
    mov     %bx, 4(%esp)
    mov     12(%eax), %bx           # copy inRegs->si to stack
    mov     %bx, 2(%esp)
    mov     14(%eax), %bx           # copy inRegs->di to stack
    mov     %bx, 0(%esp)

    // This is the encoding for `popaw`. The `popaw` mnemonic is not known
    // on old Clang versions, so we encode the instruction as raw bytes.
    //
    .byte 0x66,0x61

    pushl   %eax                    # temporarily save pop'd AX value
    movl    (4+24)(%esp), %eax      # second parameter (&outRegs)
    mov     %cx, 2(%eax)            # outRegs->cx = pop'd cx value
    mov     %dx, 4(%eax)            # outRegs->dx = pop'd dx value
    mov     %bx, 6(%eax)            # outRegs->bx = pop'd bx value
                                    # (skip pop'd sp value)
    mov     %bp, 10(%eax)           # outRegs->bp = pop'd bp value
    mov     %si, 12(%eax)           # outRegs->si = pop'd si value
    mov     %di, 14(%eax)           # outRegs->di = pop'd di value
    popl    %ebx
    mov     %bx, 0(%eax)            # outRegs->ax = pop'd ax value

    popl    %esi
    popl    %edi
    popl    %ebx
    popl    %ebp
	ret


.globl DoPopA32
#ifndef TARGET_MAC
	.type	DoPopA32,@function
#endif
DoPopA32:
	pushl	%ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi

    lea     -32(%esp), %esp         # reserve stack space for register block that will be pop'd

    movl    (32+20)(%esp), %eax     # first parameter (&inRegs)
    mov     0(%eax), %ebx           # copy inRegs->eax to stack
    mov     %ebx, 28(%esp)
    mov     4(%eax), %ebx           # copy inRegs->ecx to stack
    mov     %ebx, 24(%esp)
    mov     8(%eax), %ebx           # copy inRegs->edx to stack
    mov     %ebx, 20(%esp)
    mov     12(%eax), %ebx          # copy inRegs->ebx to stack
    mov     %ebx, 16(%esp)
    mov     16(%eax), %ebx          # copy inRegs->esp to stack
    mov     %ebx, 12(%esp)
    mov     20(%eax), %ebx          # copy inRegs->ebp to stack
    mov     %ebx, 8(%esp)
    mov     24(%eax), %ebx          # copy inRegs->esi to stack
    mov     %ebx, 4(%esp)
    mov     28(%eax), %ebx          # copy inRegs->edi to stack
    mov     %ebx, 0(%esp)

    popa

    pushl   %eax                    # temporarily save pop'd AX value
    movl    (4+24)(%esp), %eax      # second parameter (&outRegs)
    mov     %ecx, 4(%eax)           # outRegs->ecx = pop'd ecx value
    mov     %edx, 8(%eax)           # outRegs->edx = pop'd edx value
    mov     %ebx, 12(%eax)          # outRegs->ebx = pop'd ebx value
                                    # (skip pop'd esp value)
    mov     %ebp, 20(%eax)          # outRegs->ebp = pop'd ebp value
    mov     %esi, 24(%eax)          # outRegs->esi = pop'd esi value
    mov     %edi, 28(%eax)          # outRegs->edi = pop'd edi value
    popl    %ebx
    mov     %ebx, 0(%eax)           # outRegs->eax = pop'd eax value

    popl    %esi
    popl    %edi
    popl    %ebx
    popl    %ebp
	ret

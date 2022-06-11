/*
 * Copyright (C) 2008-2009 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

	.intel_syntax noprefix
	.text
# RTN of size 200000 tickled a bug in symbol handling	
.globl big
	.type	big, @function
big:
	.space 200000
	
.globl main
	.type	main, @function
main:
	call xlat
	mov ecx, 16
loop1:
	nop
	loop  loop1

	mov ecx, 16
	mov eax, 0
	cmp eax,0
loop2:
	nop
	loope  loop2

	mov ecx, 16
	mov eax, 1
	cmp eax, 0	
loop3:	
	nop
	loopne  loop3

	mov eax, 0
	ret

xlat:
	movb [table+0],0
	movb [table+1],1
	movb [table+2],2
	movb [table+256+0],7
	movb [table+256+1],8
	movb [table+256+2],9
	lea ebx,[table]
	mov eax,256+1
	xlat
	ret
	
.data
table:
	.space 512
	
	
	
		

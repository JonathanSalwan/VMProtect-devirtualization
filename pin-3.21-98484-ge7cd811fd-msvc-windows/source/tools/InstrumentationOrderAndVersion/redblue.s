/*
 * Copyright (C) 2008-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.intel_syntax noprefix
.globl main
.type main, function
main:	
	mov eax,0
	cmp eax,0
	call red
	call blue
	mov eax,1
	cmp eax,0
	call red
	call blue
	mov eax,0
	ret
	
.globl red
.type red, function
red:		
	jz r2
	jmp common
r2:
	jmp common
	
.globl blue
.type blue, function
blue:		
	jz b2
	jmp common
b2:	
	jmp common	
.globl common
.type common, function
common:		
	ret
						

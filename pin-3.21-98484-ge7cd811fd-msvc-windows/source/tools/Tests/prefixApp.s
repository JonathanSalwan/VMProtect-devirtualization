/*
 * Copyright (C) 2010-2010 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.intel_syntax noprefix
.globl main
.type	main, function
main:
	call test1
	call test2
	mov eax, 0
	ret

.globl test1
.type   test1, function
test1:
	xor eax, eax
	cmp eax, 0
	je bye
	# the following code will never executed

	# address size & segment prefixes
	addr16 mov fs:5, eax

	# rep prefix
	mov esi, mystr1
	mov edi, mystr2
	rep movsb

.globl test2
.type   test2, function
test2:
	xor eax, eax
	cmp eax, 0
	je bye
	# the following code will never executed

	# lock prefix
	lock add var, ecx

	# operand size & repne prefixes
	mov eax, 8
	mov edi, mystr3
	repne scasw

bye:
	ret

.data
var: .long 10
mystr1: .ascii "hello1"
mystr2: .ascii "hello2"
mystr3: .ascii "hello3"

/*
 * Copyright (C) 2007-2008 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.globl incinst
.type incinst, function
incinst:
	inc %rax
	ret
	
.globl andinst
.type andinst, function
andinst:
	and %rax,%rax
	ret

.global leainst
.type leainst, function
leainst:	
	# make rdi come from memory
	push %rdi
	pop %rdi
	lea 16(%rdi),%rdi
	movb $2,(%rdi)
	ret

.global leaindex
.type leaindex, function
leaindex:	
	# make index come from register
	lea 0x123(%rdi,%rdi),%rax
	mov %rax,(%rsi)
	ret
	

.global cmov_test
.type cmov_test, function
cmov_test:	
	mov $0xbeefbeef, %rax
	mov $2,%rcx
	cmp $1,%rcx
	cmove %rsp, %rax
	movl %eax,(%rdi)
	ret			

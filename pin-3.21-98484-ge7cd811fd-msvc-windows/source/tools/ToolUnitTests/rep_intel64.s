/*
 * Copyright (C) 2006-2014 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.globl main
.type main, function
main:	
	push %rcx
	push %rsi
	push %rdi

	fnop
        
	# copy instring to outstring, counting up
	mov $1, %rdx
	cld
	movq $2, %rcx
	lea instring(%rip), %rsi
	lea outstring(%rip), %rdi
	rep movsb
	
	cld
	mov $2, %rdx
	movq $2, %rcx
	lea instring(%rip), %rsi
	lea outstring(%rip), %rdi
	rep movsw
	
	cld
	mov $3, %rdx
	movq $2, %rcx
	lea instring(%rip), %rsi
	lea outstring(%rip), %rdi
	rep movsl
	
	mov $4, %rdx
	cld
	movq $2, %rcx
	lea instring(%rip), %rsi
	lea outstring(%rip), %rdi
	rep movsq

	# copy instring to outstring, counting down
	mov $5, %rdx
	std
	movq $2, %rcx
	lea instring+16-1(%rip), %rsi
	lea outstring+16-1(%rip), %rdi
	rep movsb
	
	mov $6, %rdx
	std
	movq $2, %rcx
	lea instring+16-2(%rip), %rsi
	lea outstring+16-2(%rip), %rdi
	rep movsw
	
	mov $7, %rdx
	std
	movq $2, %rcx
	lea instring+16-4(%rip), %rsi
	lea outstring+16-4(%rip), %rdi
	rep movsl
	
	mov $8, %rdx
	std
	movq $2, %rcx
	lea instring+16-8(%rip), %rsi
	lea outstring+16-8(%rip), %rdi
	rep movsq
	
	# store 'ab' 3 times, moving forward
	mov $9, %rdx
	cld
	movq $0x6261, %rax
	movq $3, %rcx
	lea  outstring(%rip), %rdi
	rep stosw
	
	# store 'cd' 3 times, moving backwards
	mov $10, %rdx
	std
	movq $0x6463, %rax
	movq $3, %rcx
	lea  outstring + 16 - 2(%rip), %rdi
	rep stosw
	
	# load 2 times, moving backwards
	mov $11, %rdx
	std
	movq $2, %rcx
	lea  instring + 16 - 4(%rip), %rsi
	rep lodsl
	
	# Find 'ab' in instring
	mov $12, %rdx
	cld
	movq $0x6261, %rax
	movq $16, %rcx
	lea  instring(%rip), %rdi
	repne scasw

	# Find first bytes not '01' in instring
	mov $13, %rdx
	cld
	movq $0x3130, %rax
	movq $16, %rcx
	lea  instring(%rip), %rdi
	repe scasw

	# Find first mismatch in instring1 and instring2
	mov $14, %rdx
	cld
	movq $16, %rcx
	lea  instring(%rip), %rsi
	lea  instring2(%rip), %rdi
	repe cmpsb

	# Find first match in instring1 and instring2
	mov $15, %rdx
	cld
	movq $16, %rcx
	lea  instring(%rip), %rsi
	lea  instring2(%rip), %rdi
	repne cmpsb

	# A zero length op to check predication
	mov $16, %rdx
	cld
	movq $0, %rcx
	lea  instring(%rip), %rsi
	lea  instring2(%rip), %rdi
	repne cmpsb
	mov $0, %rax
	
	pop %rdi
	pop %rsi
	pop %rcx
	ret
	
.data
instring:		
.ascii 	"0123456789abcdef"
instring2:
.ascii 	"0123456x89abcdef"
outstring:	
.ascii 	"0123456789abcdef"

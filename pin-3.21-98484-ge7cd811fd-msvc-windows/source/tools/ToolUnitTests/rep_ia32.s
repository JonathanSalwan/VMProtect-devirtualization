/*
 * Copyright (C) 2008-2009 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.globl main
.type main, function
main:	
	push %ecx
	push %esi
	push %edi
	
	fnop

        # copy instring to outstring, counting up
	mov $1, %edx
	cld
	movl $2, %ecx
	mov $instring, %esi
	mov $outstring, %edi
	rep movsb
	
	cld
	mov $2, %edx
	movl $2, %ecx
	mov $instring, %esi
	mov $outstring, %edi
	rep movsw
	
	cld
	mov $3, %edx
	movl $2, %ecx
	mov $instring, %esi
	mov $outstring, %edi
	rep movsl
	
	# copy instring to outstring, counting down
	mov $5, %edx
	std
	movl $2, %ecx
	mov $instring+16-1, %esi
	mov $outstring+16-1, %edi
	rep movsb
	
	mov $6, %edx
	std
	movl $2, %ecx
	mov $instring+16-2, %esi
	mov $outstring+16-2, %edi
	rep movsw
	
	mov $7, %edx
	std
	movl $2, %ecx
	mov $instring+16-4, %esi
	mov $outstring+16-4, %edi
	rep movsl
	
	
	# store 'ab' 3 times, moving forward
	mov $9, %edx
	cld
	movl $0x6261, %eax
	movl $3, %ecx
	mov  $outstring, %edi
	rep stosw
	
	# store 'cd' 3 times, moving backwards
	mov $10, %edx
	std
	movl $0x6463, %eax
	movl $3, %ecx
	mov  $outstring + 16 - 2, %edi
	rep stosw
	
	# load 2 times, moving backwards
	mov $11, %edx
	std
	movl $2, %ecx
	mov  $instring + 16 - 4, %esi
	rep lodsl
	
	# Find 'ab' in instring
	mov $12, %edx
	cld
	movl $0x6261, %eax
	movl $16, %ecx
	mov  $instring, %edi
	repne scasw

	# Find first bytes not '01' in instring
	mov $13, %edx
	cld
	movl $0x3130, %eax
	movl $16, %ecx
	mov  $instring, %edi
	repe scasw

	# Find first mismatch in instring1 and instring2
	mov $14, %edx
	cld
	movl $16, %ecx
	mov  $instring, %esi
	mov  $instring2, %edi
	repe cmpsb

	# Find first match in instring1 and instring2
	mov $15, %edx
	cld
	movl $16, %ecx
	mov  $instring, %esi
	mov  $instring2, %edi
	repne cmpsb

	# A test with a zero count
	mov $16, %edx
	cld
	movl $0, %ecx
	mov  $instring, %esi
	mov  $instring2, %edi
	repne cmpsb

	mov $0, %eax
	
	pop %edi
	pop %esi
	pop %ecx
	ret
	
.data
instring:		
.ascii 	"0123456789abcdef"
instring2:
.ascii 	"0123456x89abcdef"
outstring:	
.ascii 	"0123456789abcdef"

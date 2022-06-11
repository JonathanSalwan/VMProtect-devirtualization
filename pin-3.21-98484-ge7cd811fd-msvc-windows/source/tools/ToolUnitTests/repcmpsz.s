/*
 * Copyright (C) 2007-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains the assembly source of Pin unit test repcmpsz_tool
 */

        .data
one:
	.string	"IAMHEREE"
	strlen = . - one
two:	
	.string	"IWASHERE"

        .text
.globl _start

#ifdef TARGET_LINUX
.type _start, @function
#endif

_start:
	fnop
	cld
        xor     %ebx, %ebx                      # %ebx holds test number (used as exit code on failure)
        
# Test different string comparison
        inc     %ebx
	lea	one, %esi
	lea	two, %edi
	mov     $strlen,%ecx
	repe cmpsb
        cmp     $(strlen-2),%ecx                # Should fail at second byte
        jne     2f

# Test same string comparison
        inc     %ebx
	lea	one, %esi
	lea	one, %edi
	mov     $strlen,%ecx
	repe cmpsb 
        test    %ecx,%ecx                       # Should run full length
        jne     2f

# Test same string comparison, but with no count...
        inc     %ebx
	lea	one, %esi
	lea	one, %edi
        xor     %ecx,%ecx
	repe cmpsb 
        test    %ecx,%ecx                       # Should still be zero
        jne     2f

# Test scasd
        inc     %ebx
	movl	one, %eax
	lea	two, %edi
	scasw
	mov     %eax,%ecx

# and exit

	movl	$0,%ebx		# Linux first argument: exit code
2:  
	push    %ebx		# macOS* expects args on the stack
	push    %ebx
	movl	$0x1,%eax	# system call number (sys_exit)
	int	$0x80		# call kernel
	fnop


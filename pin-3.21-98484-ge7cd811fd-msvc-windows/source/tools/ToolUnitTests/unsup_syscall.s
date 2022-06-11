/*
 * Copyright (C) 2010-2017 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

# On linux 64 
# % gcc -nostdlib -o reptest reptest.S
# % pinbin -- reptest
#	
        .data
one:
	.string	"IAMHEREE"
	strlen = . - one
two:	
	.string	"IWASHERE"

        .text
.globl _start

_start:
	fnop
	cld
        xor     %rbx, %rbx                      # %rbx holds test number (used as exit code on failure)
        
# Test different string comparison
        inc     %rbx
	lea	one(%rip), %rsi
	lea	two(%rip), %rdi
	mov     $strlen,%rcx
	repe cmpsb
        cmp     $(strlen-2),%rcx                # Should fail at second byte
        jne     2f

# Test same string comparison
        inc     %rbx
	lea	one(%rip), %rsi
	lea	one(%rip), %rdi
	mov     $strlen,%rcx
	repe cmpsb 
        test    %rcx,%rcx                       # Should run full length
        jne     2f

# Test same string comparison, but with no count...
        inc     %rbx
	lea	one(%rip), %rsi
	lea	one(%rip), %rdi
        xor     %rcx,%rcx
	repe cmpsb 
        test    %rcx,%rcx                       # Should still be zero
        jne     2f

# Test scasd
        inc     %rbx
	mov	one(%rip), %rax
	lea	two(%rip), %rdi
	scasw
	mov     %rax,%rcx
	
# Test same string comparison, but with no count...
        inc     %rbx
        lea     one(%rip), %rsi
        lea     one(%rip), %rdi
        xor     %rcx,%rcx
        repe cmpsb
        test    %rcx,%rcx                       # Should still be zero
        jne     2f

# Test scasd
        inc     %rbx
        mov     one(%rip), %rax
        lea     two(%rip), %rdi
        scasw
        mov     %rax,%rcx

# Test a couple of zero sized operations
#
        inc     %rbx
        xor     %rcx,%rcx
        mov     %rcx,%rsi
        mov     %rcx,%rdi
        rep movsq

        inc     %rbx
        rep scasq

        inc     %rbx
        rep stosq

# and exit

        mov     $0,%rbx         # first argument: exit code
2:
        mov     $1,%rax         # system call number (sys_exit)
#if (__FreeBSD__)
        mov     %rbx,%rdi
        syscall
#elif (__linux__)
        int     $0x80
#else
# error Unknown target OS        
#endif       
        fnop
        

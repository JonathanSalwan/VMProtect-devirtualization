/*
 * Copyright (C) 2014-2016 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains the assembly source of Pin unit test rep_ip_at_ipoint_after_tool
 */
        .data
one:
    .string	"IAMHEREE"
    strlen = . - one
two:	
    .string	"IWASHERE"
three:
    .string "ABCDEF20"
four:
    .string "BCDEFG21"
    strlen2 = . - four
blanks:
    .string "          X"
    strlen3 = . - blanks
five:
    .string "12345678"
    strlen4 = . - five
six:
    .string "23456789a"
        .text


.globl DoReps
.type DoReps, @function
DoReps:
    fnop
    cld
        
# Test different string comparison
        inc  %ebx
        lea  one, %esi
        lea  two, %edi
        mov  $strlen,%ecx
        repe cmpsb
        cmp     $(strlen-2),%ecx                # Should fail at second byte
        jne     fail

# Test same string comparison
        inc  %ebx
        lea  one, %esi
        lea  one, %edi
        mov  $strlen,%ecx
        repe cmpsb 
        test    %ecx,%ecx                       # Should run full length
        jne     fail

# Test same string comparison, but with no count...
        inc  %ebx
        lea  one, %esi
        lea  one, %edi
        xor  %ecx,%ecx
        repe cmpsb 
        test    %ecx,%ecx                       # Should still be zero
        jne     fail

#  Test same string comparison limited by ecx

        inc   %ebx
        lea	  one, %esi
        lea   one, %edi
        mov   $strlen,%ecx
        dec   %ecx
        dec   %ecx
        dec   %ecx
        repe cmpsb 
        test  %ecx,%ecx                       # Should run til ecx becomes 0
        jne  fail


#  Test different comparison repne
        inc   %ebx
        lea   three, %esi
        lea   four, %edi
        mov   $strlen2,%ecx
        repne cmpsb 
        cmp   $2,%ecx                       # Should fail at secondlast byte
        jne   fail

#  Test scasb
        inc     %ebx	
        lea     blanks, %edi
        mov     $strlen3,%ecx
        mov     $32,%eax
        repe    scasb 
        cmp     $1,%ecx                       # Should fail at last byte
        jne     fail

#  Test rep movsb
        inc     %ebx
        lea     five, %esi
        lea     six, %edi
        mov     $strlen4,%ecx
        dec     %ecx      
        rep     movsb 
        cmp     $0,%ecx                       # Should succeed all
        jne     fail
    

    movl	$0,%eax
    jmp doret	
fail:	  
    movl	$1,%eax	
doret:	
    ret
        

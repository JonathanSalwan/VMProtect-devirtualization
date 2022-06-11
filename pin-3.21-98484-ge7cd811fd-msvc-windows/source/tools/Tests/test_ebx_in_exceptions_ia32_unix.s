/*
 * Copyright (C) 2012-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef TARGET_MAC
.globl _TestAccessViolations
_TestAccessViolations:
#else
.globl TestAccessViolations
    .type	TestAccessViolations, @function
TestAccessViolations:
#endif

    push %ebx
    push %ebp
    push %edi
    push %esi 
    xor %ebx, %ebx
    xor %edx, %edx

    movl  $0x1234, %eax
    movl  $0x2345, %ecx
    movl  $0xabcd, %ebp
    movl  $0xbcde, %edi
    movl  $0xcdef, %esi

    cmpxchg8b  (%edx)
    
    cmp  $0x1234, %eax
    jne ErrorLab
    cmp  $0x2345, %ecx
    jne ErrorLab
    cmp  $0, %ebx
    jne ErrorLab
    cmp  $0, %edx
    jne ErrorLab
    cmp  $0xabcd, %ebp
    jne ErrorLab
    cmp  $0xbcde, %edi
    jne ErrorLab
    cmp  $0xcdef, %esi
    jne ErrorLab
    

    movl  $0x3456, %eax
    movl  $0x4567, %ecx

    xlat
    

    cmp  $0x3456, %eax
    jne ErrorLab
    cmp  $0x4567, %ecx
    jne ErrorLab
    cmp  $0, %ebx
    jne ErrorLab
    cmp  $0, %edx
    jne ErrorLab
    cmp  $0xabcd, %ebp
    jne ErrorLab
    cmp  $0xbcde, %edi
    jne ErrorLab
    cmp  $0xcdef, %esi
    jne ErrorLab


    movl  $0x5678, %eax
    movl  $0x6789, %ecx

    cmpxchg8b  (%ebx)

    cmp  $0x5678, %eax
    jne ErrorLab
    cmp  $0x6789, %ecx
    jne ErrorLab
    cmp  $0, %ebx
    jne ErrorLab
    cmp  $0, %edx
    jne ErrorLab
    cmp  $0xabcd, %ebp
    jne ErrorLab
    cmp  $0xbcde, %edi
    jne ErrorLab
    cmp  $0xcdef, %esi
    jne ErrorLab

    movl  $1, %eax
    jmp RetLab

ErrorLab:
    movl  $0, %eax
RetLab:
    pop %esi
    pop %edi
    pop %ebp
    pop %ebx	
    ret	



/*
 * Copyright (C) 2010-2018 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#if (TARGET_IA32)
//
// Use a bit test and complement instruction.
// The fun here is that the memory address includes some bits from the
// bit number, and Pin used to get that wrong.
//
// int btc(char * data, UINT32 bitno)
//        
        .type btc, @function
        .global btc
btc:    
        movl     4(%esp),%ecx
        movl     8(%esp),%edx    
        btc      %edx, (%ecx)
        mov      $0, %eax
        jnc      1f
        mov      $1, %eax
1:      
// Ensure that nothing has broken edx, with memory operand rewriting
// that might happen, since Pin has to mask the value down...
        cmp     8(%esp),%edx
        je      2f
        or      $2,%eax         // Flag a problem!
2:      
        ret

// int btr(char * data, UINT32 bitno)
//        
        .type btr, @function
        .global btr
btr:    
        movl     4(%esp),%ecx
        movl     8(%esp),%eax
        // To give btr a more complicated address mode, we offset by
        // -4 and then add +4 back in. This tests a slightly different code path in Pin.
        lea      -4(%ecx),%ecx 
        btr      %eax, 4(%ecx)
        mov      $0, %eax
        jnc      1f
        mov      $1, %eax
1:      
        ret

// int bts(char * data, UINT32 bitno)
//        
        .type bts, @function
        .global bts
bts:    
        movl     4(%esp),%ecx
        movl     8(%esp),%eax
        bts      %eax, (%ecx)
        mov      $0, %eax
        jnc      1f
        mov      $1, %eax
1:      
        ret

// int bt(char * data, UINT32 bitno)
//        
        .type bt , @function
        .global bt 
bt :    
        movl     4(%esp),%ecx
        movl     8(%esp),%eax
        bt       %eax, (%ecx)
        mov      $0, %eax
        jnc      1f
        mov      $1, %eax
1:      
        ret

#else
// 64 bit code.
//
// Use a bit test and complement instruction.
//
// int btc(char * data, UINT32 bitno)
//        
        .type btc, @function
        .global btc
btc:
        mov     %rsi, %rcx
        btc      %rsi, (%rdi)
        mov      $0, %rax
        jnc      1f
        mov      $1, %rax
1:
        cmp     %rcx,%rsi
        je      2f
        or      $2,%rax
2:      
        ret

// int btr(char * data, UINT32 bitno)
//        
        .type btr, @function
        .global btr
btr:    
        // To give btr a more complicated address mode, we offset by
        // -4 and then add +4 back in. This tests a slightly different code path in Pin.
        lea      -4(%rdi),%rdi 
        btr      %rsi, 4(%rdi)
        mov      $0, %rax
        jnc      1f
        mov      $1, %rax
1:      
        ret

// int bts(char * data, UINT32 bitno)
//        
        .type bts, @function
        .global bts
bts:
        mov     %rsi,%rax
        // Try the w suffixed version as well.
        bts    %rax, (%rdi)
        mov     $0, %rax
        jnc     1f
        mov     $1, %rax
1:      
        ret

// int bt(char * data, UINT32 bitno)
//        
        .type bt , @function
        .global bt 
bt :    
        btl      %esi, (%rdi)
        mov      $0, %rax
        jnc      1f
        mov      $1, %rax
1:      
        ret
#endif
        

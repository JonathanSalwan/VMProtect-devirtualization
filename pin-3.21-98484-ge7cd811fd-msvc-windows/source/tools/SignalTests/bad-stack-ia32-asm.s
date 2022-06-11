/*
 * Copyright (C) 2010-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

    .text
#if defined(TARGET_MAC)
.globl _DoILLOnBadStack
_DoILLOnBadStack:
#else
.globl DoILLOnBadStack
DoILLOnBadStack:
#endif

    movl    %esp, %eax
    movl    $0, %esp
    ud2
    movl    %eax, %esp
    ret

#if defined(TARGET_MAC)
.globl _DoSigreturnOnBadStack
_DoSigreturnOnBadStack:
#else
.globl DoSigreturnOnBadStack
DoSigreturnOnBadStack:
#endif
    push    %ebp
    movl    %esp, %ebp
#if defined(TARGET_LINUX)
    movl    $0, %esp
    movl    $119, %eax      /* __NR_sigreturn */
    int     $128
#elif defined(TARGET_MAC)
    movl    $0, %esp
    movl    $0xb8, %eax     /* SYS_sigreturn */
    int     $0x80
#else
#error "Code not defined"
#endif
    movl    %ebp, %esp
    pop     %ebp
    ret

/*
 * Copyright (C) 2006-2014 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * If the stack is properly aligned, "SP-8" should be aligned on a 16-byte boundary
 * on entry to this (and any) function.  The 'movdqa' instruction below will fault
 * if the stack is not aligned this way.
 */

.text
    .align 4
#ifdef TARGET_MAC
.globl _CheckSPAlign
_CheckSPAlign:
#else
.globl CheckSPAlign
CheckSPAlign:
#endif
    add     $-24, %rsp
    movdqa  %xmm0, 0(%rsp)
    add     $24, %rsp
    ret

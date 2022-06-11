/*
 * Copyright (C) 2012-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text
.globl SetXmmScratchesFun
SetXmmScratchesFun:
    push      %ebp
    mov       %esp, %ebp
    mov       8(%ebp), %eax
    movdqu   (%eax),%xmm0
    movdqu   16(%eax),%xmm1
    movdqu   32(%eax),%xmm2
    movdqu   48(%eax),%xmm3
    movdqu   64(%eax),%xmm4
    movdqu   80(%eax),%xmm5
    movdqu   96(%eax),%xmm6
    movdqu   112(%eax),%xmm7
    pop       %ebp
    ret



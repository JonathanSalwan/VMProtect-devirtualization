/*
 * Copyright (C) 2011-2011 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.globl neg_disp
.type	neg_disp, @function
neg_disp:
    mov -24(%rdi), %rax
    ret

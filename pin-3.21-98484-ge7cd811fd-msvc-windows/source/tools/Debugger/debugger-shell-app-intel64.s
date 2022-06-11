/*
 * Copyright (C) 2009-2009 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

    .text
.globl AssemblyReturn
.globl Label_WriteAx
AssemblyReturn:
    mov     %rdi, %rax
Label_WriteAx:
    ret

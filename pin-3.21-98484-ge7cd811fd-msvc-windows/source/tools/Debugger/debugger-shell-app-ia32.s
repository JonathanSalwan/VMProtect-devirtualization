/*
 * Copyright (C) 2009-2009 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

    .text
.globl AssemblyReturn
.globl Label_WriteAx
AssemblyReturn:
    mov     4(%esp), %eax
Label_WriteAx:
    ret

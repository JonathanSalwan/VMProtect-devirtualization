/*
 * Copyright (C) 2011-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text
.align 4
.globl DoXmm
DoXmm:
    sub         $0x3c, %esp
    movdqa      %xmm2, (%esp)
    add         $0x3c, %esp
    ret



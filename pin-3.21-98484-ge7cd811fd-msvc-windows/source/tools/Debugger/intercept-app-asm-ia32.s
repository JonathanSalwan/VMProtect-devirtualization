/*
 * Copyright (C) 2010-2010 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

    .text
.globl Breakpoint2
	.type	Breakpoint2, @function
Breakpoint2:
    mov     $0, %eax

.globl Breakpoint2Label
Breakpoint2Label:
    ret
	.size	Breakpoint2, .-Breakpoint2

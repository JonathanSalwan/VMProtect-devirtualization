/*
 * Copyright (C) 2007-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

# This code returns the stack pointer.
#
.global getSP
.type getSP, function

getSP:
    mov %esp, %eax
    ret


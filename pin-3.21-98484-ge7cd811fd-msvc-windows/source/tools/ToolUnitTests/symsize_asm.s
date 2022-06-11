/*
 * Copyright (C) 2012-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.globl very_big
.type very_big, @function

very_big:
    movl $0x1234, %eax
    ret

.size very_big, -1

/*
 * Copyright (C) 2011-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text
.global probed_func_asm
.type probed_func_asm,  @function
probed_func_asm:
    xor %rax, %rax
    cmp $0, %rcx
    jne NOT_ZERO
    mov $2, %rax
NOT_ZERO:
    mov $1, %rax
    ret

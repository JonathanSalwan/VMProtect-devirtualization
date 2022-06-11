/*
 * Copyright (C) 2007-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

# This code returns the stack pointer.
#
.global NAME(getSP)
DECLARE_FUNCTION(getSP)

NAME(getSP):
    mov %esp, %eax
    ret





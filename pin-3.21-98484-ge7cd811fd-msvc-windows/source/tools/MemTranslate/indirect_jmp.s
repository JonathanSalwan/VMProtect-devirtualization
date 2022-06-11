/*
 * Copyright (C) 2014-2014 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

DECLARE_FUNCTION(main)

.global NAME(main)

NAME(main):
    lea PIC_VAR(next_line),RETURN_REG
    push RETURN_REG
    mov $0xfed,RETURN_REG
// This is a jmp with bad address, but it will be translated to "jmp *(rsp)"
    jmp *(RETURN_REG)
next_line:
    pop RETURN_REG
    mov $0,RETURN_REG
    ret


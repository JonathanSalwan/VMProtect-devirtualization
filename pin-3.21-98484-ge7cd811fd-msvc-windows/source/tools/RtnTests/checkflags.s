/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "asm_macros.h"

DECLARE_FUNCTION(CheckFlags)

.global NAME(CheckFlags)

NAME(CheckFlags):
    BEGIN_STACK_FRAME
    push    GBX_REG
    push    GCX_REG
    # Set the ZF and OF flags.
    mov     $0x80000000, RETURN_REG
    shl     $32, RETURN_REG
    shl     $1, RETURN_REG
    # Save the flags register before the analysis routine.
    pushf
    pop     GBX_REG
    mov     PARAM1, RETURN_REG
    mov     %ebx, (RETURN_REG)
    # The tool will create an artificial RTN here and add instrumentation.
    # Save the flags register after the analysis routine.
    pushf
    pop     GCX_REG
    mov     PARAM2, RETURN_REG
    mov     %ecx, (RETURN_REG)
    # Compare the flags before and after the analysis routine.
    cmp     %ebx, %ecx
    jnz     end
    # GAX is not zero since it holds a valid address on the stack. If the flags are identical, indicate success.
    mov     $0, RETURN_REG
end:
    pop     GCX_REG
    pop     GBX_REG
    END_STACK_FRAME
    ret

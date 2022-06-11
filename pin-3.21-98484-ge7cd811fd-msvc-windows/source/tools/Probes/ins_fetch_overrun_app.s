/*
 * Copyright (C) 2015-2016 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

// This function is never called

DECLARE_FUNCTION(foo)
DECLARE_FUNCTION(bar)

NAME(foo):
	BEGIN_STACK_FRAME
        END_STACK_FRAME
        call *GAX_REG
END_FUNCTION(foo)

        .byte   0x0

NAME(bar):
        BEGIN_STACK_FRAME
        mov $0x1ee7,RETURN_REG
        END_STACK_FRAME
        ret
END_FUNCTION(bar)

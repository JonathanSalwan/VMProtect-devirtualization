/*
 * Copyright (C) 2014-2014 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

DECLARE_FUNCTION(modifyFlags)

.global NAME(getFlags)
.global NAME(modifyFlags)

NAME(modifyFlags):
   ret

// Sets the flags register to the first argument of this function
// then call modifyFlags() and returns the value of the flags register
// after the invocation.
NAME(getFlags):
   BEGIN_STACK_FRAME
   pushf
   mov PARAM1, RETURN_REG
   push RETURN_REG
   popf
   call NAME(modifyFlags)
   pushf
   pop RETURN_REG
   popf
   END_STACK_FRAME
   ret


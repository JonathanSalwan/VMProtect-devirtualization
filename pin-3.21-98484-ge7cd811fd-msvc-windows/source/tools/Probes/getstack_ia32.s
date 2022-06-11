/*
 * Copyright (C) 2011-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "asm_macros.h"

.global NAME(getstack)
DECLARE_FUNCTION(getstack)

NAME(getstack):
   mov %esp, %eax
   ret

.global NAME(getebp)
DECLARE_FUNCTION(getebp)

NAME(getebp):
   mov %ebp, %eax
   ret


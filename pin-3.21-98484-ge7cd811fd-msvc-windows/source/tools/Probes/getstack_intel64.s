/*
 * Copyright (C) 2011-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "asm_macros.h"

.global NAME(getstack)
DECLARE_FUNCTION(getstack)

NAME(getstack):
   mov %rsp, %rax
   ret

.global NAME(getebp)
DECLARE_FUNCTION(getebp)

NAME(getebp):
   mov %rbp, %rax
   ret


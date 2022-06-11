/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

DECLARE_FUNCTION(Proc1)
.global NAME(Proc1)	

NAME(Proc1): 
 mov GAX_REG, GCX_REG
 shl CL_REG, GDX_REG
 ret 


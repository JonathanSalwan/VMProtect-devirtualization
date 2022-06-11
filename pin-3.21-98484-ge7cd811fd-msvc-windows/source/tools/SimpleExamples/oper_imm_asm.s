/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

DECLARE_FUNCTION(operImmCmds)

.global NAME(operImmCmds)
//.intel_syntax noprefix 

// Includes several examples of commands that include immediate operands,
// to be analysed by the tool to extract and display the operand values 
NAME(operImmCmds):
   BEGIN_STACK_FRAME
   mov PARAM1, RETURN_REG
   add $0x10, RETURN_REG
   mov $1, %al
   mov $2, %cx
   mov $3, %edx
#if defined(TARGET_IA32)
   add $-4, %ax
#else
   add $-4, %rax
#endif
   END_STACK_FRAME
   ret


/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "asm_macros.h"

#define REDZONE_SIZE 0x100

DECLARE_FUNCTION(CheckRedZone)
DECLARE_FUNCTION(InsideCheckRedZone)

.global NAME(CheckRedZone)
.global NAME(InsideCheckRedZone)

NAME(CheckRedZone):
	movq $REDZONE_SIZE/8, %rcx
	mov $0xdeadbeeffeedface, %rax
	lea -REDZONE_SIZE(%rsp), %rdi
	rep stosq
NAME(InsideCheckRedZone):
	movq $REDZONE_SIZE/8, %rcx
	mov $0xdeadbeeffeedface, %rax
	lea -REDZONE_SIZE(%rsp), %rdi
	repz scasq
	mov %rcx, %rax
	ret

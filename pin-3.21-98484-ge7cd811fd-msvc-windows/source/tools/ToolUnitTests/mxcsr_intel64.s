/*
 * Copyright (C) 2010-2014 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text
#ifdef TARGET_MAC
.globl _GetMxcsr
_GetMxcsr:
#else
.globl GetMxcsr
.type GetMxcsr, function
GetMxcsr: 
#endif
	stmxcsr (%rdi)
    ret


#ifdef TARGET_MAC
.globl _SetMxcsr
_SetMxcsr:
#else
.globl SetMxcsr
.type SetMxcsr, function
SetMxcsr: 
#endif
	ldmxcsr (%rdi)
    ret
 

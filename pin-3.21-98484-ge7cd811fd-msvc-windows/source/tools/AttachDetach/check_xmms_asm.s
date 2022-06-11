/*
 * Copyright (C) 2018-2018 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

.text

# Set XMM0-7 to 0    
DECLARE_FUNCTION_AS(CleanXmms) 
CleanXmms: 
    
    BEGIN_STACK_FRAME
    
    xorpd   %xmm0, %xmm0
	xorpd   %xmm1, %xmm1    
	xorpd   %xmm2, %xmm2
	xorpd   %xmm3, %xmm3
	xorpd   %xmm4, %xmm4
	xorpd   %xmm5, %xmm5
	xorpd   %xmm6, %xmm6
	xorpd   %xmm7, %xmm7
	
	END_STACK_FRAME
	
	ret


# Store XMM0-7 into buffer pointer by first function argument
DECLARE_FUNCTION_AS(SaveXmms) 
SaveXmms:

	BEGIN_STACK_FRAME
	
	mov       PARAM1, SCRATCH_REG1 	
	
	movdqu %xmm0, (SCRATCH_REG1) 
	movdqu %xmm1, 16(SCRATCH_REG1)
	movdqu %xmm2, 32(SCRATCH_REG1)
	movdqu %xmm3, 48(SCRATCH_REG1)
	movdqu %xmm4, 64(SCRATCH_REG1)
	movdqu %xmm5, 80(SCRATCH_REG1)
	movdqu %xmm6, 96(SCRATCH_REG1)
	movdqu %xmm7, 112(SCRATCH_REG1)
	
	END_STACK_FRAME
	
    ret

# Scramble XMM0-7 registers
DECLARE_FUNCTION_AS(ScrambleXmms)
ScrambleXmms:

	BEGIN_STACK_FRAME
#if defined(TARGET_IA32E)	
	mov $0x0001020304050607, SCRATCH_REG1
	mov $0x08090a0b0c0d0e0f, SCRATCH_REG2
    movq   SCRATCH_REG1, %xmm0
    pinsrq $1, SCRATCH_REG2, %xmm0

    movq   SCRATCH_REG1, %xmm1
    pinsrq $1, SCRATCH_REG2, %xmm1

    movq   SCRATCH_REG1, %xmm2
    pinsrq $1, SCRATCH_REG2, %xmm2

    movq   SCRATCH_REG1, %xmm3
    pinsrq $1, SCRATCH_REG2, %xmm3

    movq   SCRATCH_REG1, %xmm4
    pinsrq $1, SCRATCH_REG2, %xmm4

    movq   SCRATCH_REG1, %xmm5
    pinsrq $1, SCRATCH_REG2, %xmm5

    movq   SCRATCH_REG1, %xmm6
    pinsrq $1, SCRATCH_REG2, %xmm6

    movq   SCRATCH_REG1, %xmm7
    pinsrq $1, SCRATCH_REG2, %xmm7
#else
	mov $0x00010203, SCRATCH_REG1
	mov $0x04050607, SCRATCH_REG2
	mov $0x08090a0b, SCRATCH_REG3
	push CALLEE_SAVE_REG1
	mov $0x0c0d0e0f, CALLEE_SAVE_REG1
	
    movd   SCRATCH_REG1, %xmm0
    pinsrd $1, SCRATCH_REG2, %xmm0
    pinsrd $2, SCRATCH_REG3, %xmm0
	pinsrd $3, CALLEE_SAVE_REG1, %xmm0
	
    movd   SCRATCH_REG1, %xmm1
    pinsrd $1, SCRATCH_REG2, %xmm1
    pinsrd $2, SCRATCH_REG3, %xmm1
	pinsrd $3, CALLEE_SAVE_REG1, %xmm1

    movd   SCRATCH_REG1, %xmm2
    pinsrd $1, SCRATCH_REG2, %xmm2
    pinsrd $2, SCRATCH_REG3, %xmm2
	pinsrd $3, CALLEE_SAVE_REG1, %xmm2

    movd   SCRATCH_REG1, %xmm3
    pinsrd $1, SCRATCH_REG2, %xmm3
    pinsrd $2, SCRATCH_REG3, %xmm3
	pinsrd $3, CALLEE_SAVE_REG1, %xmm3

    movd   SCRATCH_REG1, %xmm4
    pinsrd $1, SCRATCH_REG2, %xmm4
    pinsrd $2, SCRATCH_REG3, %xmm4
	pinsrd $3, CALLEE_SAVE_REG1, %xmm4

    movd   SCRATCH_REG1, %xmm5
    pinsrd $1, SCRATCH_REG2, %xmm5
    pinsrd $2, SCRATCH_REG3, %xmm5
	pinsrd $3, CALLEE_SAVE_REG1, %xmm5

    movd   SCRATCH_REG1, %xmm6
    pinsrd $1, SCRATCH_REG2, %xmm6
    pinsrd $2, SCRATCH_REG3, %xmm6
	pinsrd $3, CALLEE_SAVE_REG1, %xmm6

    movd   SCRATCH_REG1, %xmm7
    pinsrd $1, SCRATCH_REG2, %xmm7
    pinsrd $2, SCRATCH_REG3, %xmm7
	pinsrd $3, CALLEE_SAVE_REG1, %xmm7
	
	pop CALLEE_SAVE_REG1
#endif    
    END_STACK_FRAME
    
    ret

# Do nothing
DECLARE_FUNCTION_AS(DoNothing)
DoNothing:
	
	BEGIN_STACK_FRAME
	
	nop
	
	END_STACK_FRAME
	
	ret
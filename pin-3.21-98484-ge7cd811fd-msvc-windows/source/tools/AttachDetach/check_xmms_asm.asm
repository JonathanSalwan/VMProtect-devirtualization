;
; Copyright (C) 2018-2018 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc

PROLOGUE

PUBLIC CleanXmms
PUBLIC SaveXmms
PUBLIC ScrambleXmms
PUBLIC DoNothing

.code

; Set XMM0-7 to 0
CleanXmms PROC
    
    BEGIN_STACK_FRAME
    
    xorpd   xmm0, xmm0
	xorpd   xmm1, xmm1
	xorpd   xmm2, xmm2
	xorpd   xmm3, xmm3
	xorpd   xmm4, xmm4
	xorpd   xmm5, xmm5
	xorpd   xmm6, xmm6
	xorpd   xmm7, xmm7
	
	END_STACK_FRAME
	
	ret
CleanXmms ENDP

; Store XMM0-7 into buffer pointer by first function argument
SaveXmms PROC

	BEGIN_STACK_FRAME
	
	mov       SCRATCH_REG1, PARAM1
	
	movdqu xmmword ptr [SCRATCH_REG1], xmm0
	movdqu xmmword ptr [SCRATCH_REG1]+16, xmm1
	movdqu xmmword ptr [SCRATCH_REG1]+32, xmm2
	movdqu xmmword ptr [SCRATCH_REG1]+48, xmm3
	movdqu xmmword ptr [SCRATCH_REG1]+64, xmm4
	movdqu xmmword ptr [SCRATCH_REG1]+80, xmm5
	movdqu xmmword ptr [SCRATCH_REG1]+96, xmm6
	movdqu xmmword ptr [SCRATCH_REG1]+112, xmm7
	
	END_STACK_FRAME
	
    ret
SaveXmms ENDP

; Scramble XMM0-7 registers
ScrambleXmms PROC

	BEGIN_STACK_FRAME
IFDEF TARGET_IA32E
	mov SCRATCH_REG1, 0001020304050607h
	mov SCRATCH_REG2, 08090a0b0c0d0e0fh
    pinsrq xmm0, SCRATCH_REG1, 0
    pinsrq xmm0, SCRATCH_REG2, 1

    pinsrq xmm1, SCRATCH_REG1, 0
    pinsrq xmm1, SCRATCH_REG2, 1

    pinsrq xmm2, SCRATCH_REG1, 0
    pinsrq xmm2, SCRATCH_REG2, 1

    pinsrq xmm3, SCRATCH_REG1, 0
    pinsrq xmm3, SCRATCH_REG2, 1

    pinsrq xmm4, SCRATCH_REG1, 0
    pinsrq xmm4, SCRATCH_REG2, 1

    pinsrq xmm5, SCRATCH_REG1, 0
    pinsrq xmm5, SCRATCH_REG2, 1

    pinsrq xmm6, SCRATCH_REG1, 0
    pinsrq xmm6, SCRATCH_REG2, 1

    pinsrq xmm7, SCRATCH_REG1, 0
    pinsrq xmm7, SCRATCH_REG2, 1
ELSE
	mov SCRATCH_REG1, 00010203h
	mov SCRATCH_REG2, 04050607h
	mov SCRATCH_REG3, 08090a0bh
	push CALLEE_SAVE_REG1
	mov CALLEE_SAVE_REG1, 0c0d0e0fh
	
    movd   xmm0, SCRATCH_REG1
    pinsrd xmm0, SCRATCH_REG2, 1
    pinsrd xmm0, SCRATCH_REG3, 2
	pinsrd xmm0, CALLEE_SAVE_REG1, 3
	
    movd   xmm1, SCRATCH_REG1
    pinsrd xmm1, SCRATCH_REG2, 1
    pinsrd xmm1, SCRATCH_REG3, 2
	pinsrd xmm1, CALLEE_SAVE_REG1, 3

    movd   xmm2, SCRATCH_REG1
    pinsrd xmm2, SCRATCH_REG2, 1
    pinsrd xmm2, SCRATCH_REG3, 2
	pinsrd xmm2, CALLEE_SAVE_REG1, 3

    movd   xmm3, SCRATCH_REG1
    pinsrd xmm3, SCRATCH_REG2, 1
    pinsrd xmm3, SCRATCH_REG3, 2
	pinsrd xmm3, CALLEE_SAVE_REG1, 3

    movd   xmm4, SCRATCH_REG1
    pinsrd xmm4, SCRATCH_REG2, 1
    pinsrd xmm4, SCRATCH_REG3, 2
	pinsrd xmm4, CALLEE_SAVE_REG1, 3

    movd   xmm5, SCRATCH_REG1
    pinsrd xmm5, SCRATCH_REG2, 1
    pinsrd xmm5, SCRATCH_REG3, 2
	pinsrd xmm5, CALLEE_SAVE_REG1, 3

    movd   xmm6, SCRATCH_REG1
    pinsrd xmm6, SCRATCH_REG2, 1
    pinsrd xmm6, SCRATCH_REG3, 2
	pinsrd xmm6, CALLEE_SAVE_REG1, 3

    movd   xmm7, SCRATCH_REG1
    pinsrd xmm7, SCRATCH_REG2, 1
    pinsrd xmm7, SCRATCH_REG3, 2
	pinsrd xmm7, CALLEE_SAVE_REG1, 3
	
	pop CALLEE_SAVE_REG1
ENDIF
    END_STACK_FRAME
    
    ret
ScrambleXmms ENDP

; Do nothing
DoNothing PROC
	
	BEGIN_STACK_FRAME
	
	nop
	
	END_STACK_FRAME
	
	ret
DoNothing ENDP

end

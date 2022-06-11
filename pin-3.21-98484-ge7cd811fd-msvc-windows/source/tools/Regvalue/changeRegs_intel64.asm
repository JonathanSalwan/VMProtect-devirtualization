;
; Copyright (C) 2013-2018 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC ChangeRegsWrapper
PUBLIC ChangeRegs
PUBLIC SaveRegsToMem

extern gprval:qword
extern agprval:qword
extern stval:real10
extern astval:real10
extern xmmval:xmmword
extern axmmval:xmmword
IFDEF CONTEXT_USING_AVX
extern ymmval:ymmword
extern aymmval:ymmword
ENDIF
IFDEF CONTEXT_USING_AVX512F
extern zmmval:ymmword    ;  We don't care much about the size we just need reference to the variable.
extern azmmval:ymmword   ;  since there is no support yet for zmmword this is good enough  
extern opmaskval:qword
extern aopmaskval:qword
ENDIF
extern fpSaveArea:qword

.code

; void ChangeRegsWrapper();
; This wrapper saves and restores the registers used by ChangeRegs.
; This is done in the wrapper since we want ChangeRegs to actually
; change the register values but not to affect the application itself.
; The tool may intercept ChangeRegs and replace it with its own function.
;
; Register usage:
; rax   - used (implicitly) by xsave
; rbx   - used for testing the gpr values
; rcx   - used for holding a pointer to the fp save area (used by fxsave)
; rdx   - used (implicitly) by xsave
; st0   - used (implicitly) for loading a value to the FPU stack
; st2   - used for testing the FPU values
; xmm0  - used for testing the sse (xmm) values
; ymm1  - used for testing the avx (ymm) values
; zmm5  - used for testing the avx512 (zmm) values
; k3    - used for testing the opmask register values
ChangeRegsWrapper PROC
    ; Save the necessary GPRs
    push    rax
    push    rbx
    push    rcx
    push    rdx

IFDEF CONTEXT_USING_AVX512F
    ; Save the necessary mask registers
    ;kmovw   eax, k3
	BYTE 0c5h
	BYTE 0f8h
	BYTE 093h
	BYTE 0c3h
    push    rax
ENDIF

    ; Allign the fpSaveArea
    lea     rcx, fpSaveArea
    add     rcx, 40H
    and     rcx, 0ffffffffffffffc0H
    ; Save the floating-point state
IFDEF CONTEXT_USING_AVX
    push    rdx
    xor     rdx, rdx
    mov     rax, 7
    xsave   [rcx]
ELSE
    fxsave  [rcx]
ENDIF

    ; Now call ChangeRegs - do the actual test.
    ; The tool may intercept this function and modify the register values itself.
    call    ChangeRegs

    ; Placeholder for PIN_ExecuteAt
    call    ExecuteAt

    ; Save the modified values to memory so the tool can ispect them.
    ; This is relevant only when the tool modifies the values.
    call    SaveRegsToMem

    ; Restore the floating-point state
IFDEF CONTEXT_USING_AVX
    mov     rax, 7
    xrstor  [rcx]
    pop     rdx
ELSE
    fxrstor [rcx]
ENDIF

IFDEF CONTEXT_USING_AVX512F
    ; Restore the mask registers
    pop     rax
    ;kmovw   k3, eax
	BYTE 0c5h
	BYTE 0f8h
	BYTE 092h
	BYTE 0d8h
ENDIF

    ; Restore the GPRs
    pop     rdx
    pop     rcx
    pop     rbx
    pop     rax
    ret
ChangeRegsWrapper ENDP

; void ChangeRegs();
; For register usage see ChangeRegsWrapper above.
ChangeRegs PROC
    ; TEST: load the new value to rbx
    mov     rbx, gprval
    ; prepare the test value at the top of the FPU stack
    fld     real10 ptr stval
    ; TEST: load the new value to st2
    fst     st(2)
    ; TEST: load the new value to xmm0
    movdqu  xmm0, xmmword ptr xmmval
IFDEF CONTEXT_USING_AVX
    ; TEST: load the new value to ymm1
    vmovdqu ymm1, ymmword ptr ymmval
ENDIF
IFDEF CONTEXT_USING_AVX512F
    ; TEST: load the new value to zmm5
	; instead of:
	;   simple vmovdqu64 zmm5, zmmword ptr zmmval
	; do:
	;   lea rax, zmmval
	;   vmovdqu64 zmm5,[rax]
	; encode the avx512 instruction in bytes
	; since we cannot insert relocation as assembler can we need this split of lea followed by move
	push rax
	lea rax, zmmval
	BYTE 062h
	BYTE 0f1h
	BYTE 0feh
	BYTE 048h
	BYTE 06fh
	BYTE 028h
    ; TEST: load the new value to k3
    ; instead of:
    ;    kmovw   k3, opmaskval
	; do:
	;    lea rax opmaskval
	;    kmovw k3, [rax] (encoded in bytes)
	lea rax, opmaskval
	BYTE 0c5h
	BYTE 0f8h
	BYTE 090h
	BYTE 018h
	pop rax
ENDIF
    ret
ChangeRegs ENDP

; void ExecuteAt();
ExecuteAt PROC
    ret
ExecuteAt ENDP

; void SaveRegsToMem();
; Save the necessary registers to memory.
; The tool will then compare the value stored in memory to the ones it expects to find.
; For register usage see ChangeRegsWrapper above.
SaveRegsToMem PROC
    ; TEST: store the new value of rbx
    mov     agprval, rbx
    ; prepare the test value at the top of the FPU stack
    fld     st(2)
    ; TEST: store the new value of st2
    fstp    real10 ptr astval
    ; TEST: store the new value of xmm0
    movdqu  xmmword ptr axmmval, xmm0
IFDEF CONTEXT_USING_AVX
    ; TEST: store the new value of ymm1
    vmovdqu ymmword ptr aymmval, ymm1
ENDIF
IFDEF CONTEXT_USING_AVX512F
    ; TEST: store the new value of zmm5
    ; instead of simple:
    ;    vmovdqu64 zmmword ptr azmmval, zmm5
	; do:
	;    lea rax, azmmval
	;    vmovdqu64 [rax], zmm5
	; encode the avx512 instruction in bytes
	push rax
	lea rax, azmmval
	BYTE 062h
	BYTE 0f1h
	BYTE 0feh
	BYTE 048h
	BYTE 07fh
	BYTE 028h
    ; TEST: store the new value of k3
    ; instead of:
    ;    kmovw   k3, opmaskval
	; do:
	;    lea rax aopmaskval
	;    kmovw   [rax], k3 (encoded in bytes)
	lea rax, aopmaskval 
	BYTE 0c5h
	BYTE 0f8h
	BYTE 091h
	BYTE 018h
	pop rax
ENDIF
    ret
SaveRegsToMem ENDP

end

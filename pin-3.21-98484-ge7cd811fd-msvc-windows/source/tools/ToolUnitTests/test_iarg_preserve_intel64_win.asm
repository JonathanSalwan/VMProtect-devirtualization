;
; Copyright (C) 2011-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC Fld1Fstp_a
PUBLIC Fld1Fstp_b
PUBLIC Fld1Fstp_c
PUBLIC Fld1Fstp_d
PUBLIC Fld1Fstp_e
PUBLIC Fld1Fstp_f
PUBLIC Fld1Fstp_g
PUBLIC Fld1Fstp_h
PUBLIC Fld1Fstp_i
PUBLIC Fld1Fstp_j
PUBLIC Fld1Fstp_k
PUBLIC Fld1Fstp_l
PUBLIC Fld1Fstp_m
PUBLIC Fld1Fstp_n
PUBLIC Fld1Fstp_n1
PUBLIC Fld1Fstp_o
PUBLIC Fld1Fstp_p
PUBLIC Fld1Fstp_q
PUBLIC IntScratchFld1Fstp_a
PUBLIC IntScratchFld1Fstp_b
PUBLIC IntScratchFld1Fstp_c
PUBLIC IntScratchFld1Fstp_d
PUBLIC FldzToTop3
PUBLIC SetXmmRegsToZero
PUBLIC SetIntRegsToZero
PUBLIC SetIntRegsToDeadBeef
PUBLIC get_gax
PUBLIC get_gbx
PUBLIC get_gcx
PUBLIC get_gdx
PUBLIC get_gdi
PUBLIC get_gsi
PUBLIC get_gbp
PUBLIC set_gax
PUBLIC set_gbx
PUBLIC set_gcx
PUBLIC set_gdx
PUBLIC set_gdi
PUBLIC set_gsi
PUBLIC set_gbp
PUBLIC get_r8
PUBLIC get_r9
PUBLIC get_r10
PUBLIC get_r11
PUBLIC get_r12
PUBLIC get_r13
PUBLIC get_r14
PUBLIC get_r15
PUBLIC set_r8
PUBLIC set_r9
PUBLIC set_r10
PUBLIC set_r11
PUBLIC set_r12
PUBLIC set_r13
PUBLIC set_r14
PUBLIC set_r15
PUBLIC GetMxcsr
PUBLIC MaskZeroDivideInMxcsr
PUBLIC UnMaskZeroDivideInMxcsr
PUBLIC Fld1FstpSetXmmsMaskMxcsr

PUBLIC Fld2tToTop3
PUBLIC AddToXmmRegs
PUBLIC Fld1Fstp_f1
PUBLIC Fld1Fstp_f2
PUBLIC Fld1_aa
PUBLIC Fld1_ab
PUBLIC Fld1_ac
PUBLIC Fld1_ad
PUBLIC Fld1_ae
PUBLIC Fld1_af
PUBLIC Fld1_ag
PUBLIC GetFlags
PUBLIC SetFlags
PUBLIC Fld1Fstp_l1
PUBLIC Fld1Fstp_l2
PUBLIC Fstp_x
PUBLIC Fld1_x

extern TestIargPreserveInReplacement:PROC
extern TestIargPreserveInReplacement1:PROC
extern TestIargPreserveInReplacement2:PROC
extern TestIargPreserveInProbed:PROC
extern TestIargPreserveInProbed1:PROC
extern TestIargPreserveInProbed2:PROC





extern var1:qword
extern var2:qword
extern var3:qword
extern val1:qword
extern val2:qword
extern val3:qword
extern val4:qword
extern val5:qword
extern val6:qword
extern val7:qword
extern val8:qword
extern val9:qword
extern val10:qword
extern val11:qword
extern val12:qword
extern val13:qword
extern val14:qword
extern val15:qword
extern val16:qword
extern setFlagsX:qword

.data
dummy QWORD 0, 0, 0 , 0

.code
Fld1Fstp_a PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_a ENDP

Fld1Fstp_b PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_b ENDP

Fld1Fstp_c PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_c ENDP

Fld1Fstp_d PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_d ENDP

Fld1Fstp_e PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_e ENDP

Fld1Fstp_f PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_f ENDP

Fld1Fstp_g PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_g ENDP

Fld1Fstp_h PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_h ENDP

Fld1Fstp_i PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_i ENDP

Fld1Fstp_j PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_j ENDP

Fld1Fstp_k PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_k ENDP

Fld1Fstp_l PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_l ENDP

Fld1Fstp_m PROC
    fld1
	fld1
	fld1
	call TestIargPreserveInReplacement
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_m ENDP

Fld1Fstp_n PROC
    fld1
	fld1
	fld1
	call TestIargPreserveInReplacement1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_n ENDP

Fld1Fstp_n1 PROC
    fld1
	fld1
	fld1
	call TestIargPreserveInReplacement2
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_n1 ENDP

Fld1Fstp_o PROC
    fld1
	fld1
	fld1
	call TestIargPreserveInProbed
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_o ENDP


Fld1Fstp_p PROC
    fld1
	fld1
	fld1
	call TestIargPreserveInProbed1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_p ENDP

Fld1Fstp_q PROC
    fld1
	fld1
	fld1
	call TestIargPreserveInProbed2
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_q ENDP

Fld1FstpSetXmmsMaskMxcsr PROC
    fld1
	fld1
	fld1
	call SetXmmRegsToZero
	call MaskZeroDivideInMxcsr
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1FstpSetXmmsMaskMxcsr ENDP


IntScratchFld1Fstp_a PROC
    mov rax, 0deadbeefH
	mov rcx, 0deadbeefH
	mov rdx, 0deadbeefH
	mov r8, 0deadbeefH
	mov r9, 0deadbeefH
	mov r10, 0deadbeefH
	mov r11, 0deadbeefH
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

IntScratchFld1Fstp_a ENDP

IntScratchFld1Fstp_b PROC
    mov rax, 0deadbeefH
	mov rcx, 0deadbeefH
	mov rdx, 0deadbeefH
	mov r8, 0deadbeefH
	mov r9, 0deadbeefH
	mov r10, 0deadbeefH
	mov r11, 0deadbeefH
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

IntScratchFld1Fstp_b ENDP

IntScratchFld1Fstp_c PROC
    mov rax, 0deadbeefH
	mov rcx, 0deadbeefH
	mov rdx, 0deadbeefH
	mov r8, 0deadbeefH
	mov r9, 0deadbeefH
	mov r10, 0deadbeefH
	mov r11, 0deadbeefH
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

IntScratchFld1Fstp_c ENDP

IntScratchFld1Fstp_d PROC
    mov rax, 0deadbeefH
	mov rcx, 0deadbeefH
	mov rdx, 0deadbeefH
	mov r8, 0deadbeefH
	mov r9, 0deadbeefH
	mov r10, 0deadbeefH
	mov r11, 0deadbeefH
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

IntScratchFld1Fstp_d ENDP

SetIntRegsToZero PROC
    xor rax, rax
	xor rcx, rcx
	xor rdx, rdx
	xor r8, r8
	xor r9, r9
	xor r10, r10
	xor r11, r11
	xor r12, r12
	xor r13, r13
	xor r14, r14
    ret

SetIntRegsToZero ENDP




SetIntRegsToDeadBeef PROC
    mov rax, 0deadbeefH
	mov rcx, 0deadbeefH
	mov rdx, 0deadbeefH
	mov r8, 0deadbeefH
	mov r9, 0deadbeefH
	mov r10, 0deadbeefH
	mov r11, 0deadbeefH
    ret

SetIntRegsToDeadBeef ENDP

get_gax PROC
    ret

get_gax ENDP

get_gbx PROC
    mov rax, rbx
    ret

get_gbx ENDP

get_gdi PROC
    mov rax, rdi
    ret

get_gdi ENDP

get_gsi PROC
    mov rax, rsi
    ret

get_gsi ENDP

get_gbp PROC
    mov rax, rbp
    ret

get_gbp ENDP

get_gcx PROC
    mov rax, rcx
    ret

get_gcx ENDP

get_gdx PROC
    mov rax, rdx
    ret

get_gdx ENDP

get_r8 PROC
    mov rax, r8
    ret

get_r8 ENDP

get_r9 PROC
    mov rax, r9
    ret

get_r9 ENDP

get_r10 PROC
    mov rax, r10
    ret

get_r10 ENDP

get_r11 PROC
    mov rax, r11
    ret

get_r11 ENDP


get_r12 PROC
    mov rax, r12
    ret
get_r12 ENDP

get_r13 PROC
    mov rax, r13
    ret
get_r13 ENDP

get_r14 PROC
    mov rax, r14
    ret
get_r14 ENDP

get_r15 PROC
    mov rax, r15
    ret
get_r15 ENDP


set_gax PROC
    mov rax, rcx
    ret

set_gax ENDP


set_gbx PROC
    mov rbx, rcx
    ret

set_gbx ENDP

set_gcx PROC
    mov rcx, rcx
    ret

set_gcx ENDP

set_gdx PROC
    mov rdx, rcx
    ret

set_gdx ENDP


set_gdi PROC
    mov rdi, rcx
    ret

set_gdi ENDP

set_gsi PROC
    mov rsi, rcx
    ret

set_gsi ENDP


set_gbp PROC
    mov rbp, rcx
    ret

set_gbp ENDP

set_r8 PROC
    mov r8, rcx
    ret

set_r8 ENDP

set_r9 PROC
    mov r9, rcx
    ret

set_r9 ENDP

set_r10 PROC
    mov r10, rcx
    ret

set_r10 ENDP

set_r11 PROC
    mov r11, rcx
    ret

set_r11 ENDP

set_r12 PROC
    mov r12, rcx
    ret

set_r12 ENDP

set_r13 PROC
    mov r13, rcx
    ret

set_r13 ENDP

set_r14 PROC
    mov r14, rcx
    ret

set_r14 ENDP

set_r15 PROC
    mov r15, rcx
    ret

set_r15 ENDP

FldzToTop3 PROC
    fstp dummy
	fstp dummy
	fstp dummy
	fldz
	fldz
	fldz
    ret

FldzToTop3 ENDP



SetXmmRegsToZero PROC
    pxor xmm0, xmm0
    pxor xmm1, xmm1
	pxor xmm2, xmm2
	pxor xmm3, xmm3
	pxor xmm4, xmm4
	pxor xmm5, xmm5
	pxor xmm6, xmm6
	pxor xmm7, xmm7
	pxor xmm8, xmm8
	pxor xmm9, xmm9
	pxor xmm10, xmm10
	pxor xmm11, xmm11
	pxor xmm12, xmm12
	pxor xmm13, xmm13
	pxor xmm14, xmm14
	pxor xmm15, xmm15
    
    ret
SetXmmRegsToZero ENDP

GetMxcsr PROC
   push rax
   stmxcsr [rsp]
   pop rax
   ret

GetMxcsr ENDP

MaskZeroDivideInMxcsr PROC
   call GetMxcsr
   or rax,  200H
   push rax
   ldmxcsr [rsp]
   pop rax
   ret

MaskZeroDivideInMxcsr ENDP

UnMaskZeroDivideInMxcsr PROC
   call GetMxcsr
   mov rcx, 200H
   not rcx
   and rax, rcx
   push rax
   ldmxcsr [rsp]
   pop rax
   ret

UnMaskZeroDivideInMxcsr ENDP


GetFlags PROC
    pushfq
    pop rax
    ret

GetFlags ENDP

SetFlags PROC
    push setFlagsX
    popfq
    ret
SetFlags ENDP

Fld2tToTop3 PROC
    fstp dummy
	fstp dummy
	fstp dummy
	fldl2t
	fldl2t
	fldl2t
    ret

Fld2tToTop3 ENDP

Fstp3 PROC
    fstp dummy
	fstp dummy
	fstp dummy
	ret
Fstp3 ENDP

Fstp_x PROC
    fstp dummy
	fstp dummy
	fstp dummy
	ret
Fstp_x ENDP

Fld1_x PROC
    fld1
	fld1
	fld1
	ret
Fld1_x ENDP

Fld1Fstp_f1 PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_f1 ENDP

Fld1Fstp_f2 PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_f2 ENDP

Fld1_aa PROC
    fld1
	fld1
	fld1
    ret

Fld1_aa ENDP


Fld1_ab PROC
    fld1
	fld1
	fld1
    ret

Fld1_ab ENDP

Fld1_ac PROC
    fld1
	fld1
	fld1
    ret

Fld1_ac ENDP

Fld1_ad PROC
    fld1
	fld1
	fld1
    ret

Fld1_ad ENDP


Fld1_ae PROC
    fld1
	fld1
	fld1
    ret

Fld1_ae ENDP

Fld1_af PROC
    fld1
	fld1
	fld1
    ret

Fld1_af ENDP

Fld1_ag PROC
    fld1
	fld1
	fld1
    ret

Fld1_ag ENDP


AddToXmmRegs PROC
    lea rax, val1
	mov rax, qword ptr [rax]
    paddd xmm0, xmmword ptr [rax]
	lea rax, val2
	mov rax, qword ptr [rax]
    paddd xmm1, xmmword ptr [rax]
	lea rax, val3
	mov rax, qword ptr [rax]
	paddd xmm2, xmmword ptr [rax]
	lea rax, val4
	mov rax, qword ptr [rax]
	paddd xmm3, xmmword ptr [rax]
	lea rax, val5
	mov rax, qword ptr [eax]
	paddd xmm4, xmmword ptr [eax]
	lea rax, val6
	mov rax, qword ptr [rax]
	paddd xmm5, xmmword ptr [rax]
	lea rax, val7
	mov rax, qword ptr [rax]
	paddd xmm6, xmmword ptr [rax]
	lea rax, val8
	mov rax, qword ptr [rax]
	paddd xmm7, xmmword ptr [rax]

	lea rax, val9
	mov rax, qword ptr [rax]
	paddd xmm8, xmmword ptr [rax]
	lea rax, val10
	mov rax, qword ptr [rax]
	paddd xmm9, xmmword ptr [rax]
	lea rax, val11
	mov rax, qword ptr [rax]
	paddd xmm10, xmmword ptr [rax]
	lea rax, val12
	mov rax, qword ptr [rax]
	paddd xmm11, xmmword ptr [rax]
	lea rax, val13
	mov rax, qword ptr [rax]
	paddd xmm12, xmmword ptr [rax]
	lea rax, val14
	mov rax, qword ptr [rax]
	paddd xmm13, xmmword ptr [rax]
	lea rax, val15
	mov rax, qword ptr [rax]
	paddd xmm14, xmmword ptr [rax]
	lea rax, val16
	mov rax, qword ptr [rax]
	paddd xmm15, xmmword ptr [rax]
    
    ret
AddToXmmRegs ENDP

Fld1Fstp_l1 PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_l1 ENDP

Fld1Fstp_l2 PROC
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_l2 ENDP


end



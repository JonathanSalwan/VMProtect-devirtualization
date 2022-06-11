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
PUBLIC Fld1Fstp_f
PUBLIC Fld1Fstp_f1
PUBLIC Fld1Fstp_f2
PUBLIC Fld1Fstp_g
PUBLIC Fld1Fstp_h
PUBLIC Fld1Fstp_i
PUBLIC Fld1Fstp_j
PUBLIC Fld1Fstp_k
PUBLIC Fld1Fstp_l
PUBLIC Fld1Fstp_l1
PUBLIC Fld1Fstp_l2
PUBLIC Fld1Fstp_m
PUBLIC Fld1Fstp_n
PUBLIC Fld1Fstp_o
PUBLIC Fld1Fstp_p
PUBLIC Fld1Fstp_q
PUBLIC Fld1_aa
PUBLIC Fld1_ab
PUBLIC Fld1_ac
PUBLIC Fld1_ad
PUBLIC Fld1_ae
PUBLIC Fld1_af
PUBLIC Fld1_ag
PUBLIC Fstp3
PUBLIC Fstp_x
PUBLIC Fld1_x
PUBLIC IntScratchFld1Fstp_a
PUBLIC IntScratchFld1Fstp_b
PUBLIC IntScratchFld1Fstp_c
PUBLIC IntScratchFld1Fstp_d
PUBLIC FldzToTop3
PUBLIC Fld2tToTop3
PUBLIC SetXmmRegsToZero
PUBLIC AddToXmmRegs
PUBLIC SetIntRegsToZero
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
PUBLIC GetMxcsr
PUBLIC MaskZeroDivideInMxcsr
PUBLIC UnMaskZeroDivideInMxcsr
PUBLIC Fld1FstpSetXmmsMaskMxcsr
PUBLIC GetFlags
PUBLIC SetFlags




.686
.XMM
.model flat, c
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
extern setFlagsX:dword
.data
dummy QWORD 0, 0, 0 , 0
varDeadBeef DWORD 0deadbeefH, 0deadbeefH, 0deadbeefH, 0deadbeefH

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

Fld1Fstp_m PROC
    fld1
	fld1
	fld1
	mov eax, dword ptr [esp+4]
	call eax
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_m ENDP

Fld1Fstp_n PROC
    fld1
	fld1
	fld1
	mov eax, dword ptr [esp+4]
	call eax
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_n ENDP

Fld1Fstp_o PROC
    fld1
	fld1
	fld1
	mov eax, dword ptr [esp+4]
	call eax
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_o ENDP


Fld1Fstp_p PROC
    fld1
	fld1
	fld1
	mov eax, dword ptr [esp+4]
	call eax
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_p ENDP

Fld1Fstp_q PROC
    fld1
	fld1
	fld1
	mov eax, dword ptr [esp+4]
	call eax
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_q ENDP


Fld1Fstp_n1 PROC
    fld1
	fld1
	fld1
	mov eax, dword ptr [esp+4]
	call eax
	fstp var1
	fstp var2
	fstp var3
    ret

Fld1Fstp_n1 ENDP


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
    mov eax, 0deadbeefH
	mov ecx, 0deadbeefH
	mov edx, 0deadbeefH
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

IntScratchFld1Fstp_a ENDP

IntScratchFld1Fstp_b PROC
    mov eax, 0deadbeefH
	mov ecx, 0deadbeefH
	mov edx, 0deadbeefH
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

IntScratchFld1Fstp_b ENDP

IntScratchFld1Fstp_c PROC
    mov eax, 0deadbeefH
	mov ecx, 0deadbeefH
	mov edx, 0deadbeefH
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

IntScratchFld1Fstp_c ENDP

IntScratchFld1Fstp_d PROC
    mov eax, 0deadbeefH
	mov ecx, 0deadbeefH
	mov edx, 0deadbeefH
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

IntScratchFld1Fstp_d ENDP

SetIntRegsToZero PROC
    xor eax, eax
	xor ecx, ecx
	xor edx, edx
    ret

SetIntRegsToZero ENDP

get_gax PROC
    ret

get_gax ENDP

get_gbx PROC
    mov eax, ebx
    ret
get_gbx ENDP

get_gcx PROC
    mov eax, ecx
    ret

get_gcx ENDP

get_gdx PROC
    mov eax, edx
    ret

get_gdx ENDP

get_gdi PROC
    mov eax, edi
    ret
get_gdi ENDP

get_gsi PROC
    mov eax, esi
    ret
get_gsi ENDP

get_gbp PROC
    mov eax, ebp
    ret
get_gbp ENDP


set_gax PROC
    mov eax, dword ptr [esp+4]
    ret

set_gax ENDP

set_gbx PROC
    mov ebx, dword ptr [esp+4]
    ret
set_gbx ENDP

set_gcx PROC
    mov ecx, dword ptr [esp+4]
    ret

set_gcx ENDP

set_gdx PROC
    mov edx, dword ptr [esp+4]
    ret

set_gdx ENDP

set_gdi PROC
    mov edi, dword ptr [esp+4]
    ret
set_gdi ENDP

set_gsi PROC
    mov esi, dword ptr [esp+4]
    ret
set_gsi ENDP

set_gbp PROC
    mov ebp, dword ptr [esp+4]
    ret
set_gbp ENDP

FldzToTop3 PROC
    fstp dummy
	fstp dummy
	fstp dummy
	fldz
	fldz
	fldz
    ret

FldzToTop3 ENDP


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

SetXmmRegsToZero PROC
    pxor xmm0, xmm0
    pxor xmm1, xmm1
	pxor xmm2, xmm2
	pxor xmm3, xmm3
	pxor xmm4, xmm4
	pxor xmm5, xmm5
	pxor xmm6, xmm6
	pxor xmm7, xmm7
    
    ret
SetXmmRegsToZero ENDP


AddToXmmRegs PROC
    lea eax, val1
	mov eax, dword ptr [eax]
    paddd xmm0, xmmword ptr [eax]
	lea eax, val2
	mov eax, dword ptr [eax]
    paddd xmm1, xmmword ptr [eax]
	lea eax, val3
	mov eax, dword ptr [eax]
	paddd xmm2, xmmword ptr [eax]
	lea eax, val4
	mov eax, dword ptr [eax]
	paddd xmm3, xmmword ptr [eax]
	lea eax, val5
	mov eax, dword ptr [eax]
	paddd xmm4, xmmword ptr [eax]
	lea eax, val6
	mov eax, dword ptr [eax]
	paddd xmm5, xmmword ptr [eax]
	lea eax, val7
	mov eax, dword ptr [eax]
	paddd xmm6, xmmword ptr [eax]
	lea eax, val8
	mov eax, dword ptr [eax]
	paddd xmm7, xmmword ptr [eax]
    
    ret
AddToXmmRegs ENDP

GetMxcsr PROC
   push eax
   stmxcsr [esp]
   pop eax
   ret

GetMxcsr ENDP

MaskZeroDivideInMxcsr PROC
   call GetMxcsr
   or eax,  200H
   push eax
   ldmxcsr [esp]
   pop eax
   ret

MaskZeroDivideInMxcsr ENDP

UnMaskZeroDivideInMxcsr PROC
   call GetMxcsr
   mov ecx, 200H
   not ecx
   and eax, ecx
   push eax
   ldmxcsr [esp]
   pop eax
   ret

UnMaskZeroDivideInMxcsr ENDP


GetFlags PROC
    pushfd
    pop eax
    ret

GetFlags ENDP

SetFlags PROC
    push setFlagsX
    popfd
    ret

SetFlags ENDP

end



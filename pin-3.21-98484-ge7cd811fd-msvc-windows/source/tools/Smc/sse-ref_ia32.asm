;
; Copyright (C) 2006-2015 Intel Corporation.
; SPDX-License-Identifier: MIT
;

.386
.XMM
.model flat, c

.data
dummy QWORD 0, 0, 0 , 0

.code

Fld1_a PROC
    fld1
	fld1
	fld1
    ret
Fld1_a ENDP


Fld1_b PROC
    fld1
	fld1
	fld1
    ret
Fld1_b ENDP

FldzToTop3_a PROC
    fstp dummy
	fstp dummy
	fstp dummy
	fldz
	fldz
	fldz
    ret

FldzToTop3_a ENDP

mmx_save PROC buf:DWORD
  push   eax
  mov    eax,  buf
  fxsave [eax]
  pop    eax
  RET
mmx_save ENDP

mmx_restore PROC buf:DWORD
  push    eax
  mov     eax,  buf 
  fxrstor [eax]
  pop     eax
  RET
mmx_restore ENDP

set_xmm_reg0 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm0,  [eax]
  RET
set_xmm_reg0 ENDP

get_xmm_reg0 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm0
  RET
get_xmm_reg0 ENDP

set_xmm_reg1 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm1,  [eax]
  RET
set_xmm_reg1 ENDP

get_xmm_reg1 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm1
  RET
get_xmm_reg1 ENDP

set_xmm_reg2 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm2,  [eax]
  RET
set_xmm_reg2 ENDP

get_xmm_reg2 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm2
  RET
get_xmm_reg2 ENDP

set_xmm_reg3 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm3,  [eax]
  RET
set_xmm_reg3 ENDP

get_xmm_reg3 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm3
  RET
get_xmm_reg3 ENDP

set_xmm_reg4 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm4,  [eax]
  RET
set_xmm_reg4 ENDP

get_xmm_reg4 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm4
  RET
get_xmm_reg4 ENDP

set_xmm_reg5 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm5,  [eax]
  RET
set_xmm_reg5 ENDP

get_xmm_reg5 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm5
  RET
get_xmm_reg5 ENDP

set_xmm_reg6 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm6,  [eax]
  RET
set_xmm_reg6 ENDP

get_xmm_reg6 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm6
  RET
get_xmm_reg6 ENDP

set_xmm_reg7 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm7,  [eax]
  RET
set_xmm_reg7 ENDP

get_xmm_reg7 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm7
  RET
get_xmm_reg7 ENDP
set_mmx_reg0 PROC mmx_reg:DWORD
  mov    eax,   mmx_reg
  movq   mm0,   [eax]
  RET
set_mmx_reg0 ENDP

get_mmx_reg0 PROC mmx_reg:DWORD
  mov    eax,   mmx_reg
  movq   [eax], mm0
  RET
get_mmx_reg0 ENDP

end

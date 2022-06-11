;
; Copyright (C) 2012-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC FpStack
PUBLIC FpCompute

.686
.XMM
.model flat, c

.data
dummy QWORD 0, 0, 0 , 0

.code 
FpStack PROC
    
    fld1
	fld1
	fld1
	fld1
	fld1
	fld1
	fld1
	fstp dummy
	fstp dummy
	fstp dummy
	fstp dummy
	fstp dummy
	fstp dummy
	fstp dummy
    ret

FpStack ENDP


FpCompute PROC
    
    fld1
	fld1
	fld1
	fld1
	fld1
	fld1
	fld1
	faddp
	faddp
	faddp
	faddp
	faddp
	faddp
	mov  ecx, dword ptr [esp+4]
	fstp dummy
	mov eax, dword ptr dummy
	mov edx, dword ptr dummy+4
	mov dword ptr [ecx], eax
	mov dword ptr [ecx+4], edx
    ret

FpCompute ENDP

end

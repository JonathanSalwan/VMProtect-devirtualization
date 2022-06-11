;
; Copyright (C) 2010-2010 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC MyMemCpy




.686
.XMM
.model flat, c



.code

MyMemCpy PROC
    push ebp
	mov  ebp, esp
	push esi
	push edi
	mov  esi, DWORD PTR [ebp+8h]
	mov  edi, DWORD PTR [ebp+0ch]
	mov  ecx, DWORD PTR [ebp+10h]
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
    movsb
	pop edi
	pop esi
	mov eax, 01h
	leave
    ret
MyMemCpy ENDP

end
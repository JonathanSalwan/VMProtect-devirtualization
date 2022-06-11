;
; Copyright (C) 2008-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC SegAccessRtn


.686
.model flat, c
COMMENT // replacement memory operands with segment registers
ASSUME NOTHING
.code
SegAccessRtn PROC
	push ebp
	mov  ebp, esp
	push ecx
	mov  eax, DWORD PTR [ebp+8h]
	mov  DWORD PTR fs:[10h], eax
	mov  eax, DWORD PTR fs:[10h]
	mov  ecx, 10h
	mov  eax, DWORD PTR fs:[ecx]
	mov  DWORD PTR fs:[14h], 100
	mov  ecx, 10h
	add  eax, DWORD PTR fs:[ecx + 4]	
	pop  ecx
	leave
	ret
	 

SegAccessRtn ENDP

SegAccessStrRtn PROC

	push ebp
	mov  ebp, esp
	push esi
	
	mov  eax, DWORD PTR [ebp+8h]
	mov  DWORD PTR fs:[14h], eax
	mov  esi, 14h
	lods DWORD PTR fs:[esi]
	
	pop esi
	leave
	ret

SegAccessStrRtn ENDP

dummy PROC
    nop
dummy ENDP

end







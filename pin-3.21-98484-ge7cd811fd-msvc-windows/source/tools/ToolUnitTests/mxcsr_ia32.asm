;
; Copyright (C) 2010-2010 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC GetMxcsr
PUBLIC SetMxcsr


.686
.XMM
.model flat, c

.code
GetMxcsr PROC
    push ebp
	mov  ebp, esp
	mov  eax, DWORD PTR [ebp+8h]
	stmxcsr DWORD PTR [eax]
	leave
    ret

GetMxcsr ENDP

SetMxcsr PROC
    push ebp
	mov  ebp, esp
	mov  eax, DWORD PTR [ebp+8h]
	ldmxcsr DWORD PTR [eax]
	leave
    ret

SetMxcsr ENDP

end
;
; Copyright (C) 2012-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

.686
.xmm
.model flat,c

ASSUME NOTHING

.CODE 
 ALIGN 4 
 SupportsTsx PROC
    push    ebp
    mov     ebp, esp
    BYTE 0C7h
    BYTE 0F8h
    BYTE 002h
    BYTE 000h
    BYTE 000h
    BYTE 000h

    jmp successLabel
abortLabel:
    mov eax, 0
    jmp returnLabel
successLabel:
    mov eax, 1

    BYTE 00fh
    BYTE 001h
    BYTE 0d5h

returnLabel:
    mov     esp, ebp
    pop     ebp
    ret
SupportsTsx ENDP

END
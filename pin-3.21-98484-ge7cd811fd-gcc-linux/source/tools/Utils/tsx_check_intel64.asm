;
; Copyright (C) 2012-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

.CODE 
 ALIGN 4 
 SupportsTsx PROC
    push    rbp
    mov     rbp, rsp
    push    rax
    push    rbx
    push    rcx
    push    rdx
    push    rsi       
    
    BYTE 0C7h
    BYTE 0F8h
    BYTE 002h
    BYTE 000h
    BYTE 000h
    BYTE 000h

    jmp successLabel
abortLabel:
    mov rax, 0
    jmp returnLabel
successLabel:
    mov rax, 1

    BYTE 00fh
    BYTE 001h
    BYTE 0d5h

returnLabel:
    pop    rsi
    pop    rdx
    pop    rcx
    pop    rbx

    mov     rsp, rbp
    pop     rbp
    ret
SupportsTsx ENDP

END
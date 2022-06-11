;
; Copyright (C) 2008-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC SetAppFlags_asm


.686
.model flat, c
extern flagsVal:dword

.code
SetAppFlags_asm PROC
    pushfd
    pop eax
    mov edx, eax
    or eax, 0cd5H
    push eax
    popfd
    pushfd
    pop eax
    lea ecx,flagsVal
    mov       DWORD PTR [ecx], eax
    push edx
    popfd
    ret

SetAppFlags_asm ENDP

end
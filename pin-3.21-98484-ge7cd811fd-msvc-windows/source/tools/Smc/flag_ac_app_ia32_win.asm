;
; Copyright (C) 2009-2015 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC SetAppFlags_asm
PUBLIC ClearAcFlag_asm
PUBLIC GetFlags_asm


.686
.model flat, c


.code
SetAppFlags_asm PROC

    push ebp
	mov  ebp, esp
	mov  eax, DWORD PTR [ebp+8h]
    pushfd
    pop ecx
    or ecx, eax
    mov edx, eax
    push ecx
    popfd
    leave
    ret

SetAppFlags_asm ENDP

ClearAcFlag_asm PROC

    
    pushfd
    pop ecx
    and ecx, 0fffbffffH
    push ecx
    popfd
    ret

ClearAcFlag_asm ENDP


GetFlags_asm PROC

    
    pushfd
    pop eax
    ret

GetFlags_asm ENDP

end
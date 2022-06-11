;
; Copyright (C) 2010-2021 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC SupportsAvx2


.686
.model flat, c

.code
 ALIGN 4 
 SupportsAvx2 PROC
    push    ebp
    mov     ebp, esp
    pusha

    mov     eax, 1

    cpuid

    and ecx, 0018000000h
    cmp ecx, 0018000000h 
    jne $lNOT_SUPPORTED 
    mov ecx, 0          
    
    BYTE 00Fh
    BYTE 001h
    BYTE 0D0h
    and eax, 6
    cmp eax, 6
    jne $lNOT_SUPPORTED
    mov     eax, 7
    mov ecx, 0
    cpuid
    and ebx, 020h
    cmp ebx, 020h
    jne $lNOT_SUPPORTED
    popa
    mov eax, 1
    jmp $lDONE
$lNOT_SUPPORTED:
    popa
    mov eax, 0
$lDONE:
    mov     esp, ebp
    pop     ebp
    ret
SupportsAvx2 ENDP

end

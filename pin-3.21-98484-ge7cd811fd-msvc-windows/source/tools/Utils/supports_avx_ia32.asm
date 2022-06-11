;
; Copyright (C) 2010-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

.686
.xmm
.model flat,c

ASSUME NOTHING

.CODE 
 ALIGN 4 
 ProcessorSupportsAvx PROC
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
    popa
    mov eax, 1
    jmp $lDONE3
$lNOT_SUPPORTED:
    popa
    mov eax, 0
$lDONE3:
    mov     esp, ebp
    pop     ebp
    ret
ProcessorSupportsAvx ENDP

END
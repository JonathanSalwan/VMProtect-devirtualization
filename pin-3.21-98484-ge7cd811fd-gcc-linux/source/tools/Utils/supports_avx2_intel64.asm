;
; Copyright (C) 2010-2021 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC SupportsAvx2
PUBLIC Do_Xsave
PUBLIC Do_Fxsave
PUBLIC Do_Xrstor
PUBLIC Do_Fxrstor


.code
SupportsAvx2 PROC
    push    rbp
    mov     rbp, rsp

    push    rbx
    push    rcx
    push    rdx
    push    rsi

    mov     rax, 1      

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
    mov ecx, 0          
    mov     rax, 7
    cpuid
    and ebx, 020h
    cmp ebx, 020h
    jne $lNOT_SUPPORTED  
    mov rax, 1
    jmp $lDONE3
$lNOT_SUPPORTED:
    mov rax, 0
$lDONE3:
    pop    rsi
    pop    rdx
    pop    rcx
    pop    rbx

    mov     rsp, rbp
    pop     rbp
    ret
SupportsAvx2 ENDP

.code
Do_Xsave PROC
    xor rdx,rdx
    mov rax,7
 
    ret
Do_Xsave ENDP

.code
Do_Fxsave PROC
    fxsave xmmword ptr[rcx]
    ret
Do_Fxsave ENDP

.code
Do_Xrstor PROC
    xor rdx,rdx
    mov rax,7
    
    ret
Do_Xrstor ENDP

.code
Do_Fxrstor PROC
    fxrstor xmmword ptr[rcx]
    ret
Do_Fxrstor ENDP

end

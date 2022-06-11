;
; Copyright (C) 2014-2021 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC DoExplicitMemoryOps


.686
.model flat, c
extern globalVar:dword
extern dynVar:dword
extern lblPtr:dword
extern autoVarPtr:dword

COMMENT // use of segment register is not an ERROR
ASSUME FS:NOTHING

.code
 ALIGN 4
 DoExplicitMemoryOps PROC
    push ebp
    mov ebp, esp
    sub esp, 16

lbl1:
    lea eax, globalVar

lbl2:
    lea eax, [esp + 8] ; <--- this will be autoVar

    mov ebx, [dynVar]
lbl3:
    lea eax, [ebx]

    mov eax, 0cafebabeH
lbl4:
    lea eax, [eax]

    xor eax, eax
lbl5:
    lea eax, [eax+0deadbeeH]

lbl6:
    mov eax, globalVar

lbl7:
    mov [esp + 8], eax

lbl8:
    lea eax, fs:-8  ; -8 is 32-bit segment offset

    mov eax, 0deadbeefH
lbl9:
    lea eax, fs:[eax]

    lea eax, [esp + 8]
    mov [autoVarPtr], eax

    mov ebx, [lblPtr]
    mov eax, offset lbl1
    mov [ebx], eax
    mov eax, offset lbl2
    mov [ebx+4], eax
    mov eax, offset lbl3
    mov [ebx+8], eax
    mov eax, offset lbl4
    mov [ebx+12], eax
    mov eax, offset lbl5
    mov [ebx+16], eax
    mov eax, offset lbl6
    mov [ebx+20], eax
    mov eax, offset lbl7
    mov [ebx+24], eax
    mov eax, offset lbl8
    mov [ebx+28], eax
    mov eax, offset lbl9
    mov [ebx+32], eax

    mov esp, ebp
    pop ebp
    ret
DoExplicitMemoryOps ENDP

end

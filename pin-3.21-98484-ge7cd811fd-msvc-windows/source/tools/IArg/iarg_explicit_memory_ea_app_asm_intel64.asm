;
; Copyright (C) 2014-2014 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC DoExplicitMemoryOps

.code

extern globalVar:qword
extern dynVar:qword
extern lblPtr:qword
extern autoVarPtr:qword

 DoExplicitMemoryOps PROC
    push rbp
    mov rbp, rsp
    sub rsp, 16

lbl1:
    lea rax, globalVar

lbl2:
    lea rax, [rsp + 8] ; <-- this will be autoVar

    mov rbx, [dynVar]
lbl3:
    lea rax, [rbx]

    mov rax, 0cafebabeH
lbl4:
    lea rax, [rax]

lbl5:
    lea rax, [0deadbeeH]
lbl6:
    mov rax, globalVar

lbl7:
    mov [rsp + 8], rax

    lea rax, [rsp + 8]
    mov [autoVarPtr], rax

lbl8:
    lea rax, fs:[-8]

    mov rax, 0deadbeefH
lbl9:
    lea rax, fs:[rax]

    mov rbx, [lblPtr]
    mov rax, offset lbl1
    mov [rbx], rax
    mov rax, offset lbl2
    mov [rbx+8], rax
    mov rax, offset lbl3
    mov [rbx+16], rax
    mov rax, offset lbl4
    mov [rbx+24], rax
    mov rax, offset lbl5
    mov [rbx+32], rax
    mov rax, offset lbl6
    mov [rbx+40], rax
    mov rax, offset lbl7
    mov [rbx+48], rax
    mov rax, offset lbl8
    mov [rbx+56], rax
    mov rax, offset lbl9
    mov [rbx+64], rax

    mov rsp, rbp
    pop rbp
    ret
DoExplicitMemoryOps ENDP

end

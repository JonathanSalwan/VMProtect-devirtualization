;
; Copyright (C) 2014-2014 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC UpdateIcountByAdd
PUBLIC UpdateIcountByInc
PUBLIC UpdateIcountByDecInc
PUBLIC UpdateIcountBySub
PUBLIC IfFuncWithAddThatCannotBeChangedToLea

.code


UpdateIcountByAdd PROC
    
    mov rax, qword ptr [rcx]
	add rax, 1
	mov qword ptr [rcx], rax
    ret
UpdateIcountByAdd ENDP

UpdateIcountByInc PROC
    
    mov rax, qword ptr [rcx]
	inc rax
	mov qword ptr [rcx], rax
    ret
UpdateIcountByInc ENDP

UpdateIcountByDecInc PROC
    
    mov rax, qword ptr [rcx]
	dec rax
	inc rax
	inc rax
	mov qword ptr [rcx], rax
    ret
UpdateIcountByDecInc ENDP

UpdateIcountBySub PROC
    
    mov rax, qword ptr [rcx]
	sub rax, -1
	mov qword ptr [rcx], rax
    ret
UpdateIcountBySub ENDP

IfFuncWithAddThatCannotBeChangedToLea PROC
    xor eax, eax  
    add eax, 1 
    setz al 
    ret
IfFuncWithAddThatCannotBeChangedToLea ENDP

end

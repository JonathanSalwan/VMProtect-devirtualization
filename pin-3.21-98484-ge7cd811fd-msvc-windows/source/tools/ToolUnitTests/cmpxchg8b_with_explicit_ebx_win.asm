;
; Copyright (C) 2008-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC cmpxchg8_with_explicit_ebx


.686
.model flat, c
extern eaxVal:dword
extern edxVal:dword
extern a:dword
.code
cmpxchg8_with_explicit_ebx PROC

    lea ecx,a
    mov ebx, ecx
    mov eax, 0
    mov edx, 0
    cmpxchg8b QWORD PTR [ebx+eax]
    lea ecx,eaxVal
    mov DWORD PTR [ecx], eax
    lea ecx,edxVal
    mov DWORD PTR [ecx], edx
    
    ret

cmpxchg8_with_explicit_ebx ENDP


	
end

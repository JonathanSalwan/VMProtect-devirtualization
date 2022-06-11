;
; Copyright (C) 2007-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC main_asm


.686
.model flat, c
extern addcVal:word

.code
main_asm PROC
    mov eax,-1
    mov edx,0
    add eax,1
    adc edx,0
    lea ecx,addcVal
    mov       DWORD PTR [ecx], edx
    ret

main_asm ENDP

end
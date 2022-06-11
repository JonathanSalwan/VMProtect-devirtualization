;
; Copyright (C) 2017-2017 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC iretTest
PUBLIC iret_func

.686
.XMM
.model flat,c
.code

iret_func PROC
        mov eax,-1
        iretd

iret_func ENDP

iretTest PROC
        ; We have to build the stack frame ourselves
        sub     esp,12
        mov     eax,0
        mov     [esp+8],eax         ; Write the flags to zero
        mov     eax,cs
        mov     [esp+4],eax
        lea     eax,here
        mov     [esp+0],eax
        jmp     iret_func
here:   
        ret
iretTest ENDP

end

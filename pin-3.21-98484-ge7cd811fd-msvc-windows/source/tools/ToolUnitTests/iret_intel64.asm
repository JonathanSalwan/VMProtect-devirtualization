;
; Copyright (C) 2017-2017 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC iretTest
PUBLIC iret_func

.code

iret_func PROC
        mov rax,-2
        iretq

iret_func ENDP

iretTest PROC
        push    rbx
        ; Move the stack pointer down, so that we can check that the stack pointer
        ; is correctly restored by the iretq
        mov     rbx,rsp
        sub     rsp,80
        mov     rax,ss
        push    rax
        push    rbx    ; Restored stack pointer
        pushfq
        mov     rax,cs
        push    rax
        call    iret_func
        pop     rbx
        ret
iretTest ENDP

end

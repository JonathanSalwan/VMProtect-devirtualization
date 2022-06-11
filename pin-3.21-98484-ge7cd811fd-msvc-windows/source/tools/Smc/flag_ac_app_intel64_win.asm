;
; Copyright (C) 2009-2015 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC SetAppFlags_asm
PUBLIC ClearAcFlag_asm
PUBLIC GetFlags_asm




.code
SetAppFlags_asm PROC
    pushfq
    pop rax
    or rax, rcx
    push rax
    popfq
    ret


SetAppFlags_asm ENDP

ClearAcFlag_asm PROC   
    pushfq
    pop rcx
    and rcx, 0fffbffffH
    push rcx
    popfq
    ret

ClearAcFlag_asm ENDP
GetFlags_asm PROC
    pushfq
    pop rax
    ret
GetFlags_asm ENDP

end
;
; Copyright (C) 2008-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC ZeroAppFlags_asm

.686
.model flat, c
.code
ZeroAppFlags_asm PROC
    pushfd
    pop eax
    and eax, 0fffff326H
    push eax
    popfd
    ret

ZeroAppFlags_asm ENDP

end
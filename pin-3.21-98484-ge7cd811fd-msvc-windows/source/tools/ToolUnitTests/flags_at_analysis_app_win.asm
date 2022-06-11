;
; Copyright (C) 2010-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC SetOfFlag_asm


.686
.model flat, c


.code
SetOfFlag_asm PROC
    xor eax, eax
    inc eax
    pushfd
    popfd
    cmp al, 081H
    xor ecx, ecx
    ret

SetOfFlag_asm ENDP

end
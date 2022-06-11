;
; Copyright (C) 2007-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC WriteFlags_asm

.686
.model flat, c
.code
WriteFlags_asm PROC
    mov       eax, 0
    add       eax, eax
    ret

WriteFlags_asm ENDP

end
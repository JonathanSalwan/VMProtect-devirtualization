;
; Copyright (C) 2007-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC ReadFlags_asm

.686
.model flat, c
.code
ReadFlags_asm PROC
    pushfd       
    pop        eax
    ret

ReadFlags_asm ENDP

end
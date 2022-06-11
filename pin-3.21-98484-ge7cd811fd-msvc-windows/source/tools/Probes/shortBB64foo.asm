;
; Copyright (C) 2008-2008 Intel Corporation.
; SPDX-License-Identifier: MIT
;

_TEXT SEGMENT
foo PROC
    test ecx, ecx
    jz   L
    mov  eax, 2
    ret
L:  mov  eax, 5
    ret
foo ENDP
END
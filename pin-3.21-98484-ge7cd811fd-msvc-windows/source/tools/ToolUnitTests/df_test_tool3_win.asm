;
; Copyright (C) 2007-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC TestDfByMovsd


.686
.model flat, c
extern source:word
extern dest:word
.code
TestDfByMovsd PROC
    push       esi
    push       edi
    lea        esi, source+4
    lea        edi, dest+4
    movsd
    movsd
    pop        edi
    pop        esi
    ret

TestDfByMovsd ENDP

end
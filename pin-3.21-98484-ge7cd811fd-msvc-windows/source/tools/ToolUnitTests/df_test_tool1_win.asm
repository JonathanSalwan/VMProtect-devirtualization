;
; Copyright (C) 2007-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC TestDfByReadFlags


.686
.model flat, c
extern numTimesDfIsSet:word
.code
TestDfByReadFlags PROC
    pushfd       
    pop        eax
    and        eax, 1024
    shr        eax, 10
    lea        ecx, numTimesDfIsSet
    add        DWORD PTR [ecx], eax
    ret

TestDfByReadFlags ENDP

end
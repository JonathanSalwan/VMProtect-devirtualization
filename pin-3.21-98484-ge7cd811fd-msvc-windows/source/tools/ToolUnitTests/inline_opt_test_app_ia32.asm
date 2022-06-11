;
; Copyright (C) 2010-2010 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC GetIntegerScratches




.686
.XMM
.model flat, c

extern scratchVals:dword

.code

GetIntegerScratches PROC
    push esi
    lea esi, scratchVals
    mov dword ptr [esi], eax
    mov dword ptr [esi]+4, ecx
    mov dword ptr [esi]+8, edx
    pop esi
    ret
GetIntegerScratches ENDP

end
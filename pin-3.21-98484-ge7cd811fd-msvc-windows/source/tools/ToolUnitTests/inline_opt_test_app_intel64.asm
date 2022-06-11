;
; Copyright (C) 2010-2010 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC GetIntegerScratches




extern scratchVals:dword

.code





GetIntegerScratches PROC
    push rsi
    lea rsi, scratchVals
    mov qword ptr [rsi], rax
    mov qword ptr [rsi]+8, rcx
    mov qword ptr [rsi]+16, rdx
    mov qword ptr [rsi]+24, r8
    mov qword ptr [rsi]+32, r9
    mov qword ptr [rsi]+40, r10
    mov qword ptr [rsi]+48, r11
    pop rsi
    ret
GetIntegerScratches ENDP

end

;
; Copyright (C) 2012-2015 Intel Corporation.
; SPDX-License-Identifier: MIT
;

.686
.XMM
.model flat, c


PUBLIC SetXmmScratchesFun


extern xmmInitVals:dword

.code
SetXmmScratchesFun PROC
    push   ebp
    mov    ebp, esp
    lea    eax,xmmInitVals
    movdqu xmm0, xmmword ptr [eax]
    movdqu xmm1, xmmword ptr [eax]+16
    movdqu xmm2, xmmword ptr [eax]+32
    movdqu xmm3, xmmword ptr [eax]+48
    movdqu xmm4, xmmword ptr [eax]+64
    movdqu xmm5, xmmword ptr [eax]+80
    movdqu xmm6, xmmword ptr [eax]+96
    movdqu xmm7, xmmword ptr [eax]+112    
    pop    ebp
    ret
SetXmmScratchesFun ENDP



end
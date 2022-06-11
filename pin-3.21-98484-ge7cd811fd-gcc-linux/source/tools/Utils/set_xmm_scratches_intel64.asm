;
; Copyright (C) 2009-2015 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC SetXmmScratchesFun
extern xmmInitVals:dword

.code
SetXmmScratchesFun PROC
    lea rax,xmmInitVals
    movdqu xmm0, xmmword ptr [rax]
    movdqu xmm1, xmmword ptr [rax]+32
    movdqu xmm2, xmmword ptr [rax]+64
    movdqu xmm3, xmmword ptr [rax]+96
    movdqu xmm4, xmmword ptr [rax]+128
    movdqu xmm5, xmmword ptr [rax]+160
    
    ret
SetXmmScratchesFun ENDP

end
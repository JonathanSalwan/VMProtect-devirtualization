;
; Copyright (C) 2010-2010 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC ZeroOutScratches
PUBLIC SetIntegerScratchesTo1







.code
ZeroOutScratches PROC
    
    xor rax, rax
    xor rcx, rcx
    xor rdx, rdx
    xor r8, r8
    xor r9, r9
    xor r10, r10
    xor r11, r11
    pxor xmm0, xmm0
    pxor xmm1, xmm1
    pxor xmm2, xmm2
    pxor xmm3, xmm3
    pxor xmm4, xmm4
    pxor xmm5, xmm5
    ret

ZeroOutScratches ENDP


SetIntegerScratchesTo1 PROC
    xor rax, rax
    inc rax
    xor rdx, rdx
    inc rdx
    xor rcx, rcx
    inc rcx
    xor r8, r8
    inc r8
    xor r9, r9
    inc r9
    xor r10, r10
    inc r10
    xor r11, r11
    inc r11
    ret

SetIntegerScratchesTo1 ENDP




end
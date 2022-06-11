;
; Copyright (C) 2017-2018 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc

PROLOGUE

.code

loadYmm0 PROC
    mov dword ptr [esp]+4, ecx

    ; This is "VMOVDQU ymm0, YMMWORD PTR [ecx]".  We directly specify the machine code,
    ; so this test runs even when the compiler doesn't support AVX.
    db 0C5h, 0FEh, 06Fh, 001h

    ret
loadYmm0 ENDP

loadZmm0 PROC
    mov dword ptr [esp]+4, ecx

    ; This is "VMOVUPD zmm0, ZMMWORD PTR [ecx]".  We directly specify the machine code,
    ; so this test runs even when the compiler doesn't support AVX512.
    db 062h, 0F1h, 0FDh, 048h, 010h, 001h

    ret
loadZmm0 ENDP

loadK0 PROC
    mov dword ptr [esp]+4, ecx

    ; This is "KMOVW k0, WORD PTR [ecx]".  We directly specify the machine code,
    ; so this test runs even when the compiler doesn't support AVX512.
    db 0C5h, 0F8h, 090h, 001h

    ret
loadK0 ENDP

end

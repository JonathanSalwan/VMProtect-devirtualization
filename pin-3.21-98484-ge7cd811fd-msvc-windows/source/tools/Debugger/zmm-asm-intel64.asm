;
; Copyright (C) 2017-2018 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc

PROLOGUE

.code

loadYmm0 PROC
    ; This is "VMOVDQU ymm0, YMMWORD PTR [rdi]".  We directly specify the machine code,
    ; so this test runs even when the compiler doesn't support AVX.
    db 0C5h, 0FEh, 06Fh, 007h

    ret
loadYmm0 ENDP

loadZmm0 PROC
    ; This is "VMOVUPD zmm0, ZMMWORD PTR [rdi]".  We directly specify the machine code,
    ; so this test runs even when the compiler doesn't support AVX512.
    db 062h, 0F1h, 0FDh, 048h, 010h, 007h

    ret
loadZmm0 ENDP

loadK0 PROC
    ; This is "KMOVW k0, WORD PTR [rdi]".  We directly specify the machine code,
    ; so this test runs even when the compiler doesn't support AVX512.
    db 0C5h, 0F8h, 090h, 007h

    ret
loadK0 ENDP

end

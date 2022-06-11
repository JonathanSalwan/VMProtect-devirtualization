;
; Copyright (C) 2011-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC probed_func_asm


.code
probed_func_asm PROC
    xor rax, rax
    cmp rcx, 0
    jne $lNOT_ZERO
    mov rax, 2
$lNOT_ZERO:
    mov rax, 1
    ret

probed_func_asm ENDP

end
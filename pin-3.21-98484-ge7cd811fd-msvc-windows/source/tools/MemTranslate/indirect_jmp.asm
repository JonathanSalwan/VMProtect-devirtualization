;
; Copyright (C) 2014-2014 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc

PROLOGUE

PUBLIC main

.code

main PROC
    lea RETURN_REG,next_line
    push RETURN_REG
    mov RETURN_REG,0fedh 
; This is a jmp with bad address, but it will be translated to "jmp *(rsp)"
    jmp ADDRINT_PTR [RETURN_REG]
next_line:
    pop RETURN_REG
    mov RETURN_REG,0
    ret
main ENDP

end

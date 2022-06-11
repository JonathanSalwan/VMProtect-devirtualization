;
; Copyright (C) 2015-2016 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc

PROLOGUE                          

.code
PUBLIC foo                        
foo PROC
        BEGIN_STACK_FRAME
        END_STACK_FRAME
        call GAX_REG
foo ENDP
        BYTE 0h
PUBLIC bar
bar PROC
        BEGIN_STACK_FRAME
        mov  RETURN_REG, 1ee7h
        END_STACK_FRAME
        ret
bar ENDP

END

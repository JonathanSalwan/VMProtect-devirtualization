;
; Copyright (C) 2014-2014 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc

PROLOGUE

PUBLIC getFlags
PUBLIC modifyFlags

.code

modifyFlags PROC EXPORT
    ret
modifyFlags ENDP

; Sets the flags register to the first argument of this function
; then call modifyFlags() and returns the value of the flags register
; after the invocation.
getFlags PROC
    BEGIN_STACK_FRAME
    NATIVE_SIZE_SUFFIX pushf
    mov RETURN_REG, PARAM1
    push RETURN_REG
    NATIVE_SIZE_SUFFIX popf
    call modifyFlags
    NATIVE_SIZE_SUFFIX pushf
    pop RETURN_REG
    NATIVE_SIZE_SUFFIX popf
    END_STACK_FRAME
    ret
getFlags ENDP

end

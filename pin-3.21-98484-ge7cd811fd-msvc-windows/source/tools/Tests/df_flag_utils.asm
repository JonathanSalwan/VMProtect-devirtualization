;
; Copyright (C) 2015-2015 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc

PROLOGUE

PUBLIC setdf
PUBLIC cleardf

.code

setdf PROC
    std
    ret
setdf ENDP

cleardf PROC
    cld
    ret
cleardf ENDP

end

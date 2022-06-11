;
; Copyright (C) 2015-2015 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc

PROLOGUE

extern xsaveArea:dword

.code
Do_Fxsave PROC
    lea    GCX_REG, xsaveArea
    fxsave [GCX_REG]
    ret
Do_Fxsave ENDP

.code
Do_Fxrstor PROC
    lea     GCX_REG, xsaveArea
    fxrstor [GCX_REG]
    fxrstor  [xsaveArea]
    ret
Do_Fxrstor ENDP

end

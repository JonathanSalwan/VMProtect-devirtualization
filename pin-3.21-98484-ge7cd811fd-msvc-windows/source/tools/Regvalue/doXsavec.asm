;
; Copyright (C) 2015-2015 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc

PROLOGUE



extern xsaveArea:qword
extern flags:qword

.code

; void DoXsavec();
; This function calls xsave and stores the FP state in the given dst area.
; The caller is expected to allocate enough space for the xsave area.
; The function expects the given dst pointer to be properly aligned for the xsave instruction.
DoXsavec PROC

    lea     GCX_REG, flags
    mov     GAX_REG, [GCX_REG]
    lea     GCX_REG, xsaveArea
    xor     GDX_REG, GDX_REG

    ; Do xsave
    xsavec   [GCX_REG]

    ret
DoXsavec ENDP

extern xsaveArea:qword

.code

; void DoXsaveOpt();
; This function calls xsave and stores the FP state in the given dst area.
; The caller is expected to allocate enough space for the xsave area.
; The function expects the given dst pointer to be properly aligned for the xsave instruction.
DoXsaveOpt PROC

    lea     GCX_REG, flags
    mov     GAX_REG, [GCX_REG]
    lea     GCX_REG, xsaveArea
    xor     GDX_REG, GDX_REG

    ; Do xsaveopt
    xsaveopt   [GCX_REG]

    ret
DoXsaveOpt ENDP

; void DoXrstor();
; This function calls xrstor and restores the specified thetures from the xsave dst area.
; The function expects the given dst pointer to be properly aligned
DoXrstor PROC

    lea     GCX_REG, flags
    mov     GAX_REG, [GCX_REG]
    lea     GCX_REG, xsaveArea
    xor     GDX_REG, GDX_REG

    ; Do xsaveopt
    xsaveopt   [GCX_REG]

    ret
DoXrstor ENDP

end

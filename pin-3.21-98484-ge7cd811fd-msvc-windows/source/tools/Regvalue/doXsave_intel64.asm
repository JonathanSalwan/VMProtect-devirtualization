;
; Copyright (C) 2014-2017 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC DoXsave

extern xsaveArea:qword
extern flags:qword

.code

; void DoXsave();
; This function calls xsave and stores the FP state in the given dst area.
; The caller is expected to allocate enough space for the xsave area.
; The function expects the given dst pointer to be properly aligned for the xsave instruction.
DoXsave PROC

    lea     rcx, flags
    mov     rax, [rcx]
    lea     rcx, xsaveArea
    xor     rdx, rdx

    ; Do xsave
    xsave   [rcx]

    ret
DoXsave ENDP

PUBLIC DoXsave

extern xsaveArea:qword

.code

; void DoXsaveOpt();
; This function calls xsave and stores the FP state in the given dst area.
; The caller is expected to allocate enough space for the xsave area.
; The function expects the given dst pointer to be properly aligned for the xsave instruction.
DoXsaveOpt PROC

    lea     rcx, flags
    mov     rax, [rcx]
    lea     rcx, xsaveArea
    xor     rdx, rdx

    ; Do xsaveopt
    xsaveopt   [rcx]

    ret
DoXsaveOpt ENDP

; void DoXrstor();
; This function calls xrstor and restores the specified thetures from the xsave dst area.
; The function expects the given dst pointer to be properly aligned
DoXrstor PROC

    lea     rcx, flags
    mov     rax, [rcx]
    lea     rcx, xsaveArea
    xor     rdx, rdx

    ; Do xrstor
    xrstor   [rcx]

    ret
DoXrstor ENDP

; void DoFxsave();
; This function calls fxsave and stores the legacy FP state in the given dst area.
; The caller is expected to allocate enough space for the xsave area.
; The function expects the given dst pointer to be properly aligned for the fxsave instruction.
DoFxsave PROC

    lea     rcx, xsaveArea

    ; Do fxsave
    fxsave   [rcx]

    ret
DoFxsave ENDP

; void Dofxrstor();
; This function calls fxrstor and restores legacy state from the xsave dst area.
; The function expects the given dst pointer to be properly aligned
DoFxrstor PROC

    lea     rcx, xsaveArea

    ; Do fxrstor
    fxrstor   [rcx]

    ret
DoFxrstor ENDP


end

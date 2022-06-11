;
; Copyright (C) 2017-2017 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc

PROLOGUE

.data

extern funcPtr:ADDRINT_TYPE

.code

; void ClFlushFunc();
; This function calls clflush
ClFlushFunc PROC

    mov       GCX_REG ,funcPtr
    ;clflush (GCX_REG)
    db 00Fh, 0AEh, 039h

    ret
ClFlushFunc ENDP

; void ClFlushOptFunc();
; This function calls clflushopt
ClFlushOptFunc PROC

    mov       GCX_REG ,funcPtr
    ;clflushopt (GCX_REG)
    db 066h, 00Fh, 0AEh, 039h

    ret
ClFlushOptFunc ENDP

; void ClwbFunc();
; This function calls clwb
ClwbFunc PROC

    mov       GCX_REG ,funcPtr
    ;clwb (GCX_REG)
    db 066h, 00Fh, 0AEh, 031h

    ret
ClwbFunc ENDP

end

;
; Copyright (C) 2011-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC AtomicIncrement

extern numthreadsStarted:dword
.code
AtomicIncrement PROC
    lea rcx, numthreadsStarted
    inc DWORD PTR [rcx]
    ret
AtomicIncrement ENDP


end

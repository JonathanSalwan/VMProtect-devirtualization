;
; Copyright (C) 2011-2011 Intel Corporation.
; SPDX-License-Identifier: MIT
;

.code
SetMxcsr PROC
    push    rcx
    ldmxcsr [rsp]
    pop     rcx
    ret
SetMxcsr ENDP

GetMxcsr PROC
    push    rax
    stmxcsr [rsp]
    pop     rax
    ret
GetMxcsr ENDP

end

;
; Copyright (C) 2007-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

;
;If the stack is properly aligned, "SP-8" should be aligned on a 16-byte boundary
;on entry to this (and any) function.  The 'movdqa' instruction below will fault
;if the stack is not aligned this way.


.code
CheckSPAlign PROC

    add     rsp, -24
    movdqa  [rsp], xmm0
    add     rsp, 24
    ret
CheckSPAlign ENDP

end
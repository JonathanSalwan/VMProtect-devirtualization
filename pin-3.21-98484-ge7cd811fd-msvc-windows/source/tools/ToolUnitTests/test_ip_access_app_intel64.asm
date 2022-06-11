;
; Copyright (C) 2008-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC TestIpRead
PUBLIC TestIpWrite
PUBLIC Dummy


.code
TestIpRead PROC

    mov        eax, DWORD PTR [mylabelR]
mylabelR:
    
    ret

TestIpRead ENDP

TestIpWrite PROC

    mov        BYTE PTR [mylabelW], 90H
mylabelW:
    nop
    
    ret

TestIpWrite ENDP


Dummy PROC

    ret

Dummy ENDP

end

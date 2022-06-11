;
; Copyright (C) 2007-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC GetEspBeforeCalled


.code

GetEspBeforeCalled PROC
    mov        rax, rsp
    add        rax, 8
    ret  
GetEspBeforeCalled ENDP
    


end
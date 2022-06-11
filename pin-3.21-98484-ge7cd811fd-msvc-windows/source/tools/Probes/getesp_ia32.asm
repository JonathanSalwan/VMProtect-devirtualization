;
; Copyright (C) 2007-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC GetEspBeforeCalled

.686
.model flat, c
ASSUME NOTHING
.code

GetEspBeforeCalled PROC
    mov        eax, esp
    add        eax, 4
    ret  
GetEspBeforeCalled ENDP
    


end
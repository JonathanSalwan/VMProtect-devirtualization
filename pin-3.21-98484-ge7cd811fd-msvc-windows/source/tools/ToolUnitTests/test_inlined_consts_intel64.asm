;
; Copyright (C) 2011-2011 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC GetConsts




extern uint32Glob:qword
extern addrIntGlob:qword
.code
GetConsts PROC
    mov r8, rcx
    lea r9, uint32Glob
    mov QWORD PTR [r9], r8
    mov r10, rdx
    lea r11, addrIntGlob
    mov QWORD PTR [r11], r10
    ret


GetConsts ENDP



end
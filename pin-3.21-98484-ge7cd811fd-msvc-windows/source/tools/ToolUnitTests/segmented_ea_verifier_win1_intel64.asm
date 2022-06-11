;
; Copyright (C) 2007-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC TestSegmentedEA


.code
TestSegmentedEA PROC
    mov        rax, 18
    mov        rcx, 2
    mov        rdx, QWORD PTR gs:[5]
    mov        QWORD PTR gs:[5], rdx
    mov        rdx, QWORD PTR gs:[rax]
    mov        QWORD PTR gs:[rax], rdx
    mov        rdx, QWORD PTR gs:[rax+5]
    mov        QWORD PTR gs:[rax+5], rdx
    mov        rdx, QWORD PTR gs:[rcx*2]
    mov        QWORD PTR gs:[rcx*2], rdx
    mov        rdx, QWORD PTR gs:[rcx*2+5]
    mov        QWORD PTR gs:[rcx*2+5], rdx
    mov        rdx, QWORD PTR gs:[rax+rcx]
    mov        QWORD PTR gs:[rax+rcx], rdx
    mov        rdx, QWORD PTR gs:[rax+rcx*2+5]
    mov        QWORD PTR gs:[rax+rcx*2+5], rdx
    ret

TestSegmentedEA ENDP

end

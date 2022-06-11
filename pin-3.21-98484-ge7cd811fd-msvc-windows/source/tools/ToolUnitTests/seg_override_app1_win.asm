;
; Copyright (C) 2007-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC TestSegOverride


.686
.model flat, c
extern source:word
extern dest:word
COMMENT // use of segment register is not an ERROR
ASSUME NOTHING
.code
TestSegOverride PROC
    push       esi
    push       edi
    lea        esi, source
    lea        edi, dest
    push       fs
    push       es
    pop        fs
    mov        eax, DWORD PTR fs:[esi]
    pop        fs
    mov        DWORD PTR [edi], eax
    mov        eax, DWORD PTR fs:[0]
    add        esi, 4
    add        edi, 4 
    mov        eax, DWORD PTR [esi]
    mov        DWORD PTR [edi], eax
    pop        edi
    pop        esi
    ret

TestSegOverride ENDP

end
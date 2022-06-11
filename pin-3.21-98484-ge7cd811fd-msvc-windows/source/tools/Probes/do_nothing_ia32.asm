;
; Copyright (C) 2008-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

.686
.model flat, c

COMMENT // contains important code patterns
COMMENT // doesn't actually do anything
COMMENT // make sure the relocated entry will copy more than one basic block.

ASSUME NOTHING
.code
PUBLIC do_nothing
do_nothing PROC

    test eax, eax
    je lab1

    xor eax, eax
    xor eax, eax

lab1:
    xor eax, eax
    xor eax, eax

    ret
do_nothing ENDP

COMMENT // make sure the relocated entry will correctly handle a jump as the last
COMMENT // instruction in the basic block.

PUBLIC nothing_doing
nothing_doing PROC
    test eax, eax
    test eax, eax
    je lab2

    xor eax, eax
    xor eax, eax

lab2:
    xor eax, eax
    xor eax, eax

    ret
nothing_doing ENDP

COMMENT // call should be replaced with a push/jmp when relocated.

PUBLIC call_function
call_function PROC

    push ebx
    call do_nothing
    pop ebx
    ret
call_function ENDP

COMMENT // make sure this code pattern works

PUBLIC call_nothing
call_nothing PROC
    push eax
    mov  eax, eax
    push ebx
    call do_nothing
    pop  ebx
    pop  eax
    ret
call_nothing ENDP

END

;
; Copyright (C) 2008-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC main_asm


.686
.model flat, c

.code
main_asm PROC
    std
    cld
    pushfd
    popfd
    xor eax,eax
    adc eax,eax
    ret

main_asm ENDP

end
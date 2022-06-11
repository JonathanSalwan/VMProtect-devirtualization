;
; Copyright (C) 2009-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC Analysis_func
PUBLIC Analysis_func_immed
PUBLIC Analysis_func_reg_overwrite



.code
Analysis_func PROC
    push      rbx
    push      rsi
    push      rdi
    sub       rsp, 16
    mov       QWORD PTR[rsp], rbx
    mov       QWORD PTR[rsp+8], rcx
    mov       rbx, QWORD PTR [rsp+80]
    mov       ebx, DWORD PTR [rsp+80]
    mov       bx,  WORD PTR [rsp+80]
    mov       bl,  BYTE PTR [rsp+80]
    cmp       rbx, QWORD PTR [rsp+80]
    cmp       ebx, DWORD PTR [rsp+80]
    cmp       bx,  WORD PTR [rsp+80]
    cmp       bl,  BYTE PTR [rsp+80]
    cmp       QWORD PTR [rsp+80], rbx
    cmp       DWORD PTR [rsp+80], ebx
    cmp       WORD PTR [rsp+80], bx
    cmp       BYTE PTR [rsp+80], bl
    add       rbx, QWORD PTR [rsp+80]
    add       ebx, DWORD PTR [rsp+80]
    add       bx,  WORD PTR [rsp+80]
    add       bl,  BYTE PTR [rsp+80]
    adc       rbx, QWORD PTR [rsp+80]
    adc       ebx, DWORD PTR [rsp+80]
    adc       bx,  WORD PTR [rsp+80]
    adc       bl,  BYTE PTR [rsp+80]
    sub       rbx, QWORD PTR [rsp+80]
    sub       ebx, DWORD PTR [rsp+80]
    sub       bx,  WORD PTR [rsp+80]
    sub       bl,  BYTE PTR [rsp+80]
    sbb       rbx, QWORD PTR [rsp+80]
    sbb       ebx, DWORD PTR [rsp+80]
    sbb       bx,  WORD PTR [rsp+80]
    sbb       bl,  BYTE PTR [rsp+80]
    xor       rbx, QWORD PTR [rsp+80]
    xor       ebx, DWORD PTR [rsp+80]
    xor       bx,  WORD PTR [rsp+80]
    xor       bl,  BYTE PTR [rsp+80]
    or        rbx, QWORD PTR [rsp+80]
    or        ebx, DWORD PTR [rsp+80]
    or        bx,  WORD PTR [rsp+80]
    or        bl,  BYTE PTR [rsp+80]
    movzx     rbx, WORD PTR [rsp+80]
    movzx     rbx, BYTE PTR [rsp+80]
    movzx     ebx, BYTE PTR [rsp+80]
    movzx     bx, BYTE PTR [rsp+80]
    movsx     rbx, WORD PTR [rsp+80]
    movsx     rbx, BYTE PTR [rsp+80]
    movsx     ebx, BYTE PTR [rsp+80]
    movsx     bx, BYTE PTR [rsp+80]
    mov       rbx, QWORD PTR[rsp]
    mov       rcx, QWORD PTR[rsp+8] 
    add       rsp, 16
    pop rdi
    pop rsi
    pop rbx
    ret

Analysis_func ENDP

Analysis_func_immed PROC
    push      rbx
    mov       rbx, QWORD PTR [rsp+48]
    mov       ebx, DWORD PTR [rsp+48]
    mov       bx,  WORD PTR [rsp+48]
    mov       bl,  BYTE PTR [rsp+48]
    cmp       rbx, QWORD PTR [rsp+48]
    cmp       ebx, DWORD PTR [rsp+48]
    cmp       bx,  WORD PTR [rsp+48]
    cmp       bl,  BYTE PTR [rsp+48]
    cmp       QWORD PTR [rsp+48], rbx
    cmp       DWORD PTR [rsp+48], ebx
    cmp       WORD PTR [rsp+48],  bx
    cmp       BYTE PTR [rsp+48],  bl
    cmp       QWORD PTR [rsp+48], 0baadf00dH
    cmp       DWORD PTR [rsp+48], 0baadf00dH
    cmp       WORD PTR [rsp+48],  0baadH
    cmp       BYTE PTR [rsp+48],  0baH
    add       rbx, QWORD PTR [rsp+48]
    add       ebx, DWORD PTR [rsp+48]
    add       bx,  WORD PTR [rsp+48]
    add       bl,  BYTE PTR [rsp+48]
    adc       rbx, QWORD PTR [rsp+48]
    adc       ebx, DWORD PTR [rsp+48]
    adc       bx,  WORD PTR [rsp+48]
    adc       bl,  BYTE PTR [rsp+48]
    sub       rbx, QWORD PTR [rsp+48]
    sub       ebx, DWORD PTR [rsp+48]
    sub       bx,  WORD PTR [rsp+48]
    sub       bl,  BYTE PTR [rsp+48]
    sbb       rbx, QWORD PTR [rsp+48]
    sbb       ebx, DWORD PTR [rsp+48]
    sbb       bx,  WORD PTR [rsp+48]
    sbb       bl,  BYTE PTR [rsp+48]
    xor       rbx, QWORD PTR [rsp+48]
    xor       ebx, DWORD PTR [rsp+48]
    xor       bx,  WORD PTR [rsp+48]
    xor       bl,  BYTE PTR [rsp+48]
    or        rbx, QWORD PTR [rsp+48]
    or        ebx, DWORD PTR [rsp+48]
    or        bx,  WORD PTR [rsp+48]
    or        bl,  BYTE PTR [rsp+48]
    movzx     rbx,  WORD PTR [rsp+48]
    movzx     rbx,  BYTE PTR [rsp+48]
    movzx     ebx,  BYTE PTR [rsp+48]
    movzx     bx,  BYTE PTR [rsp+48]
    movsx     rbx,  WORD PTR [rsp+48]
    movsx     rbx,  BYTE PTR [rsp+48]
    movsx     bx,  BYTE PTR [rsp+48]
    movsx     ebx,  BYTE PTR [rsp+48]
    pop rbx
    ret

Analysis_func_immed ENDP

Analysis_func_reg_overwrite PROC
    push      rbx
    push      rsi
    push      rdi
    sub       rsp, 16
    mov       esi, eax
    mov       QWORD PTR[rsp], rbx
    mov       QWORD PTR[rsp+8], rcx
    mov       rbx, QWORD PTR [rsp+80]
    mov       ebx, DWORD PTR [rsp+80]
    mov       bx,  WORD PTR [rsp+80]
    mov       bl,  BYTE PTR [rsp+80]
    cmp       rbx, QWORD PTR [rsp+80]
    cmp       ebx, DWORD PTR [rsp+80]
    cmp       bx,  WORD PTR [rsp+80]
    cmp       bl,  BYTE PTR [rsp+80]
    cmp       QWORD PTR [rsp+80], rbx
    cmp       DWORD PTR [rsp+80], ebx
    cmp       WORD PTR [rsp+80], bx
    cmp       BYTE PTR [rsp+80], bl
    add       rbx, QWORD PTR [rsp+80]
    add       ebx, DWORD PTR [rsp+80]
    add       bx,  WORD PTR [rsp+80]
    add       bl,  BYTE PTR [rsp+80]
    adc       rbx, QWORD PTR [rsp+80]
    adc       ebx, DWORD PTR [rsp+80]
    adc       bx,  WORD PTR [rsp+80]
    adc       bl,  BYTE PTR [rsp+80]
    sub       rbx, QWORD PTR [rsp+80]
    sub       ebx, DWORD PTR [rsp+80]
    sub       bx,  WORD PTR [rsp+80]
    sub       bl,  BYTE PTR [rsp+80]
    sbb       rbx, QWORD PTR [rsp+80]
    sbb       ebx, DWORD PTR [rsp+80]
    sbb       bx,  WORD PTR [rsp+80]
    sbb       bl,  BYTE PTR [rsp+80]
    xor       rbx, QWORD PTR [rsp+80]
    xor       ebx, DWORD PTR [rsp+80]
    xor       bx,  WORD PTR [rsp+80]
    xor       bl,  BYTE PTR [rsp+80]
    or        rbx, QWORD PTR [rsp+80]
    or        ebx, DWORD PTR [rsp+80]
    or        bx,  WORD PTR [rsp+80]
    or        bl,  BYTE PTR [rsp+80]
    movzx     rbx, WORD PTR [rsp+80]
    movzx     rbx, BYTE PTR [rsp+80]
    movzx     bx, BYTE PTR [rsp+80]
    movzx     ebx, BYTE PTR [rsp+80]
    movsx     rbx, WORD PTR [rsp+80]
    movsx     rbx, BYTE PTR [rsp+80]
    movsx     bx, BYTE PTR [rsp+80]
    movsx     ebx, BYTE PTR [rsp+80]
    mov       rbx, QWORD PTR[rsp]
    mov       rcx, QWORD PTR[rsp+8] 
    add       rsp, 16
    pop rdi
    pop rsi
    pop rbx
    ret

Analysis_func_reg_overwrite ENDP

end
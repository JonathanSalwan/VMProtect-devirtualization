;
; Copyright (C) 2009-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC Analysis_func
PUBLIC Analysis_func_immed
PUBLIC Analysis_func_reg_overwrite


.686
.model flat, c
.code
Analysis_func PROC
    push      ebx
    push      esi
    push      edi
    sub       esp, 8
    mov       DWORD PTR[esp], ebx
    mov       DWORD PTR[esp+4], ecx
    mov       ebx, DWORD PTR [esp+18H]
    mov       bx,  WORD PTR [esp+18H]
    mov       bl,  BYTE PTR [esp+18H]
    cmp       ebx, DWORD PTR [esp+18H]
    cmp       bx,  WORD PTR [esp+18H]
    cmp       bl,  BYTE PTR [esp+18H]
    cmp       DWORD PTR [esp+18H], ebx
    cmp       WORD PTR [esp+18H], bx
    cmp       BYTE PTR [esp+18H], bl
    add       ebx, DWORD PTR [esp+18H]
    add       bx,  WORD PTR [esp+18H]
    add       bl,  BYTE PTR [esp+18H]
    adc       ebx, DWORD PTR [esp+18H]
    adc       bx,  WORD PTR [esp+18H]
    adc       bl,  BYTE PTR [esp+18H]
    sub       ebx, DWORD PTR [esp+18H]
    sub       bx,  WORD PTR [esp+18H]
    sub       bl,  BYTE PTR [esp+18H]
    sbb       ebx, DWORD PTR [esp+18H]
    sbb       bx,  WORD PTR [esp+18H]
    sbb       bl,  BYTE PTR [esp+18H]
    xor       ebx, DWORD PTR [esp+18H]
    xor       bx,  WORD PTR [esp+18H]
    xor       bl,  BYTE PTR [esp+18H]
    or        ebx, DWORD PTR [esp+18H]
    or        bx,  WORD PTR [esp+18H]
    or        bl,  BYTE PTR [esp+18H]
    movzx     ebx, WORD PTR [esp+18H]
    movzx     ebx, BYTE PTR [esp+18H]
    movzx     bx, BYTE PTR [esp+18H]
    movsx     ebx, WORD PTR [esp+18H]
    movsx     ebx, BYTE PTR [esp+18H]
    movsx     bx, BYTE PTR [esp+18H]
    mov       ebx, DWORD PTR[esp]
    mov       ecx, DWORD PTR[esp+4] 
    add       esp, 8
    pop edi
    pop esi
    pop ebx
    ret

Analysis_func ENDP

Analysis_func_immed PROC
    push      ebx
    mov       ebx, DWORD PTR [esp+8]
    mov       bx,  WORD PTR [esp+8]
    mov       bl,  BYTE PTR [esp+8]
    cmp       ebx, DWORD PTR [esp+8]
    cmp       bx,  WORD PTR [esp+8]
    cmp       bl,  BYTE PTR [esp+8]
    cmp       DWORD PTR [esp+8], ebx
    cmp       WORD PTR [esp+8],  bx
    cmp       BYTE PTR [esp+8],  bl
    cmp       DWORD PTR [esp+8], 0baadf00dH
    cmp       WORD PTR [esp+8],  0baadH
    cmp       BYTE PTR [esp+8],  0baH
    add       ebx, DWORD PTR [esp+8]
    add       bx,  WORD PTR [esp+8]
    add       bl,  BYTE PTR [esp+8]
    adc       ebx, DWORD PTR [esp+8]
    adc       bx,  WORD PTR [esp+8]
    adc       bl,  BYTE PTR [esp+8]
    sub       ebx, DWORD PTR [esp+8]
    sub       bx,  WORD PTR [esp+8]
    sub       bl,  BYTE PTR [esp+8]
    sbb       ebx, DWORD PTR [esp+8]
    sbb       bx,  WORD PTR [esp+8]
    sbb       bl,  BYTE PTR [esp+8]
    xor       ebx, DWORD PTR [esp+8]
    xor       bx,  WORD PTR [esp+8]
    xor       bl,  BYTE PTR [esp+8]
    or        ebx, DWORD PTR [esp+8]
    or        bx,  WORD PTR [esp+8]
    or        bl,  BYTE PTR [esp+8]
    movzx     ebx,  WORD PTR [esp+8]
    movzx     ebx,  BYTE PTR [esp+8]
    movzx     bx,  BYTE PTR [esp+8]
    movsx     ebx,  WORD PTR [esp+8]
    movsx     ebx,  BYTE PTR [esp+8]
    movsx     bx,  BYTE PTR [esp+8]
    pop ebx
    ret

Analysis_func_immed ENDP

Analysis_func_reg_overwrite PROC
    push      ebx
    push      esi
    push      edi
    sub       esp, 8
    mov       esi, eax
    mov       DWORD PTR[esp], ebx
    mov       DWORD PTR[esp+4], ecx
    mov       ebx, DWORD PTR [esp+18H]
    mov       bx,  WORD PTR [esp+18H]
    mov       bl,  BYTE PTR [esp+18H]
    cmp       ebx, DWORD PTR [esp+18H]
    cmp       bx,  WORD PTR [esp+18H]
    cmp       bl,  BYTE PTR [esp+18H]
    cmp       DWORD PTR [esp+18H], ebx
    cmp       WORD PTR [esp+18H], bx
    cmp       BYTE PTR [esp+18H], bl
    add       ebx, DWORD PTR [esp+18H]
    add       bx,  WORD PTR [esp+18H]
    add       bl,  BYTE PTR [esp+18H]
    adc       ebx, DWORD PTR [esp+18H]
    adc       bx,  WORD PTR [esp+18H]
    adc       bl,  BYTE PTR [esp+18H]
    sub       ebx, DWORD PTR [esp+18H]
    sub       bx,  WORD PTR [esp+18H]
    sub       bl,  BYTE PTR [esp+18H]
    sbb       ebx, DWORD PTR [esp+18H]
    sbb       bx,  WORD PTR [esp+18H]
    sbb       bl,  BYTE PTR [esp+18H]
    xor       ebx, DWORD PTR [esp+18H]
    xor       bx,  WORD PTR [esp+18H]
    xor       bl,  BYTE PTR [esp+18H]
    or        ebx, DWORD PTR [esp+18H]
    or        bx,  WORD PTR [esp+18H]
    or        bl,  BYTE PTR [esp+18H]
    movzx     ebx, WORD PTR [esp+18H]
    movzx     ebx, BYTE PTR [esp+18H]
    movzx     bx, BYTE PTR [esp+18H]
    movsx     ebx, WORD PTR [esp+18H]
    movsx     ebx, BYTE PTR [esp+18H]
    movsx     bx, BYTE PTR [esp+18H]
    mov       ebx, DWORD PTR[esp]
    mov       ecx, DWORD PTR[esp+4] 
    add       esp, 8
    pop edi
    pop esi
    pop ebx
    ret

Analysis_func_reg_overwrite ENDP

end
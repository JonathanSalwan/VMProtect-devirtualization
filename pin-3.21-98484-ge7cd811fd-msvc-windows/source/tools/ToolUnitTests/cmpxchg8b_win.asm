;
; Copyright (C) 2006-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC cmpxchg8_base
PUBLIC cmpxchg8_plus8

.686
.model flat, c
.code
cmpxchg8_base PROC
    push ebp
    mov ebp, esp

    push ebx
    push ecx
    push edx
    push esi

    mov esi, [ ebp + 8 ]
    mov eax, 1
    mov edx, 1
    mov ebx, 2
    mov ecx, 2

    cmpxchg8b QWORD PTR [esi]
    jz success1

fail1:
    mov eax, 0
    jmp end1

success1:
    mov eax, 1

end1:
    pop esi
    pop edx
    pop ecx
    pop ebx
    leave
    ret

cmpxchg8_base ENDP

cmpxchg8_plus8 PROC
    push ebp
    mov ebp, esp

    push ebx
    push ecx
    push edx
    push esi

    mov esi, [ ebp + 8 ]
    mov eax, 1
    mov edx, 1
    mov ebx, 2
    mov ecx, 2

    cmpxchg8b QWORD PTR [ esi + 8 ]
    jz success2

fail2:
    mov eax, 0
    jmp end2

success2:
    mov eax, 1

end2:
    pop esi
    pop edx
    pop ecx
    pop ebx
    leave
    ret

cmpxchg8_plus8 ENDP

cmpxchg8_esp PROC
    push ebp
    mov ebp, esp

    push ebx
    push ecx
    push edx
    push esi

    mov eax, 1
    mov edx, 1
	
    lea esp,[esp + 8]
    mov [esp],eax
    mov [esp+4],edx
	
    mov ebx, 2
    mov ecx, 2

    cmpxchg8b QWORD PTR [ esp ]
    jz success3

fail3:
    mov eax, 0
    jmp end3

success3:
    mov eax, 1

end3:
    lea esp,[esp-8]
    pop esi
    pop edx
    pop ecx
    pop ebx
    leave
    ret

cmpxchg8_esp ENDP
	
end

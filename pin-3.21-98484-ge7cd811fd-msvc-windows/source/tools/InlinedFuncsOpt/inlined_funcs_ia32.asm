;
; Copyright (C) 2014-2016 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC IfProc1
PUBLIC IfProc2
PUBLIC IfProc3
PUBLIC IfProc4
PUBLIC IfProc5
PUBLIC IfProc6
PUBLIC IfProc7
PUBLIC IfProc8
PUBLIC IfProc9
PUBLIC IfProc10
PUBLIC IfProc11
PUBLIC IfProc12
PUBLIC IfProc13
PUBLIC IfProc14
PUBLIC IfProc15
PUBLIC IfProc16
PUBLIC IfProc17
PUBLIC IfProc18
PUBLIC IfProc19
PUBLIC IfProc20
PUBLIC IfProc21
PUBLIC IfProc22
PUBLIC IfProc23
PUBLIC IfProc24
PUBLIC IfProc25
PUBLIC IfProc26
PUBLIC IfProc27
PUBLIC IfProc28
PUBLIC IfProc29
PUBLIC IfProc30
PUBLIC IfProc31
PUBLIC IfProc32
PUBLIC IfProc33
PUBLIC IfProc34
PUBLIC IfProc35
PUBLIC IfProc36
PUBLIC IfProc37
PUBLIC IfProc38
PUBLIC IfProc39
PUBLIC IfProc40

.686
.model flat, c
.XMM

.data
extern globVal:dword

.code


IfProc1 PROC
 mov edx, 1
 xor eax, eax 
 cmp edx, ecx 
 setz al 
 ret 
IfProc1 ENDP

IfProc2 PROC
 mov edx, 1
 xor eax, eax 
 cmp edx, ecx 
 setz al 
 ret 
IfProc2 ENDP

IfProc3 PROC
 lea edx, globVal   
 mov edx, dword ptr [edx] 
 xor eax, eax 
 cmp edx, ecx 
 setz al 
 ret
IfProc3 ENDP

IfProc4 PROC
 lea eax, globVal   
 mov edx, dword ptr [eax] 
 xor eax, eax 
 cmp ecx, edx 
 setz al 
 ret
IfProc4 ENDP

IfProc5 PROC   
 xor eax, eax 
 cmp edx, ecx 
 setz al 
 ret 
IfProc5 ENDP

IfProc6 PROC  
 xor eax, eax 
 cmp dl, cl 
 setz al 
 ret 
IfProc6 ENDP

IfProc7 PROC
 cmp edx, ecx
 setz al 
 movzx eax, al
 ret 
IfProc7 ENDP

IfProc8 PROC
 lea ecx, dword ptr [ecx+70h] 
 and ecx, 0fh 
 mov eax, ecx 
 ret
IfProc8 ENDP


IfProc9 PROC
 mov ecx, eax
 lea    edx, globVal   
 mov eax, dword ptr [edx]
 xor ecx, ecx 
 ret
IfProc9 ENDP


IfProc10 PROC 
 xor eax, eax 
 cmp dl, cl 
 setb al 
 ret 
IfProc10 ENDP

IfProc11 PROC
 cmp edx, ecx
 setb al 
 movzx eax, al
 ret 
IfProc11 ENDP

IfProc12 PROC 
 xor eax, eax 
 cmp dl, cl 
 setbe al 
 ret 
IfProc12 ENDP

IfProc13 PROC
 cmp edx, ecx
 setbe al 
 movzx eax, al
 ret 
IfProc13 ENDP


IfProc14 PROC 
 xor eax, eax 
 cmp dl, cl 
 setnl al 
 ret 
IfProc14 ENDP

IfProc15 PROC
 cmp edx, ecx
 setnl al 
 movzx eax, al
 ret 
IfProc15 ENDP

IfProc16 PROC 
 xor eax, eax 
 cmp dl, cl 
 setl al 
 ret 
IfProc16 ENDP

IfProc17 PROC
 cmp edx, ecx
 setl al 
 movzx eax, al
 ret 
IfProc17 ENDP

IfProc18 PROC 
 xor eax, eax 
 cmp dl, cl 
 setle al 
 ret 
IfProc18 ENDP

IfProc19 PROC
 cmp edx, ecx
 setle al 
 movzx eax, al
 ret 
IfProc19 ENDP

IfProc20 PROC 
 xor eax, eax 
 cmp dl, cl 
 setnb al 
 ret 
IfProc20 ENDP

IfProc21 PROC
 cmp edx, ecx
 setnb al 
 movzx eax, al
 ret 
IfProc21 ENDP

IfProc22 PROC 
 xor eax, eax 
 cmp dl, cl 
 setnbe al 
 ret 
IfProc22 ENDP

IfProc23 PROC
 cmp edx, ecx
 setnbe al 
 movzx eax, al
 ret 
IfProc23 ENDP

IfProc24 PROC 
 xor eax, eax 
 cmp dl, cl 
 setnle al 
 ret 
IfProc24 ENDP

IfProc25 PROC
 cmp edx, ecx
 setnle al 
 movzx eax, al
 ret 
IfProc25 ENDP


IfProc26 PROC 
 xor eax, eax 
 add dl, cl 
 seto al 
 ret 
IfProc26 ENDP

IfProc27 PROC
 add dl, cl
 seto al 
 movzx eax, al
 ret 
IfProc27 ENDP

IfProc28 PROC 
 xor eax, eax 
 add dl, cl 
 setno al 
 ret 
IfProc28 ENDP

IfProc29 PROC
 add dl, cl
 setno al 
 movzx eax, al
 ret 
IfProc29 ENDP

IfProc30 PROC 
 xor eax, eax 
 add dl, cl 
 setp al 
 ret 
IfProc30 ENDP

IfProc31 PROC
 add dl, cl
 setp al 
 movzx eax, al
 ret 
IfProc31 ENDP

IfProc32 PROC 
 xor eax, eax 
 add dl, cl 
 setnp al 
 ret 
IfProc32 ENDP

IfProc33 PROC
 add dl, cl
 setnp al 
 movzx eax, al
 ret 
IfProc33 ENDP

IfProc34 PROC 
 xor eax, eax 
 add dl, cl 
 sets al 
 ret 
IfProc34 ENDP

IfProc35 PROC
 add dl, cl
 sets al 
 movzx eax, al
 ret 
IfProc35 ENDP

IfProc36 PROC 
 xor eax, eax 
 add dl, cl 
 setns al 
 ret 
IfProc36 ENDP

IfProc37 PROC
 add dl, cl
 setns al 
 movzx eax, al
 ret 
IfProc37 ENDP

IfProc38 PROC 
 xor eax, eax 
 add dl, cl 
 setnz al 
 ret 
IfProc38 ENDP

IfProc39 PROC
 add dl, cl
 setnz al 
 movzx eax, al
 ret 
IfProc39 ENDP

IfProc40 PROC
 mov eax, 01h 
 xor eax, eax 
 cmp eax, 01h
 setz al 
 ret 
IfProc40 ENDP

end

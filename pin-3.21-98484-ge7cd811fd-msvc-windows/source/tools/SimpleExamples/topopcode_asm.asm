;
; Copyright (C) 2021-2021 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc


PROLOGUE

PUBLIC fxsaveWrap
PUBLIC fxrstorWrap

.code

fxsaveWrap PROC
   push rax

   mov rax, PARAM1
   fxsave [rax]

   pop rax
   ret
fxsaveWrap ENDP




fxrstorWrap PROC
   push rax

   mov rax, PARAM1
   fxrstor [rax]

   pop rax
   ret
fxrstorWrap ENDP


end

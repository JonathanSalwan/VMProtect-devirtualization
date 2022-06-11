;
; Copyright (C) 2015-2015 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc

PROLOGUE

PUBLIC operImmCmds

.code

; Includes several examples of commands that include immediate operands,
; to be analysed by the tool to extract and display the operand values 
operImmCmds PROC
   BEGIN_STACK_FRAME
   mov RETURN_REG, PARAM1
   add RETURN_REG, 10h
   mov al, 1
   mov cx, 2
   mov edx, 3
ifdef TARGET_IA32
   add ax, -4
else
   add rax, -4
endif
   END_STACK_FRAME
   ret
operImmCmds ENDP

end

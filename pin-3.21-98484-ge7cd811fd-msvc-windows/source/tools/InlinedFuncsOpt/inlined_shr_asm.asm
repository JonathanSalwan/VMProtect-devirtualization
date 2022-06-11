;
; Copyright (C) 2015-2015 Intel Corporation.
; SPDX-License-Identifier: MIT
;

include asm_macros.inc

PROLOGUE

PUBLIC Proc1


.code


Proc1 PROC
 mov GCX_REG, GAX_REG
 shl GDX_REG, CL_REG 
 ret 
Proc1 ENDP



end

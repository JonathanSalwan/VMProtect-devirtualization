;
; Copyright (C) 2014-2014 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC main

.686
.model flat,c

; This assembly file should built to an executable file
; It tests the correctness of jcxz instruction and exits
; with status 0 if everything's OK.

.code

main PROC
   mov ecx, 10000H
   xor eax,eax
   jcxz test_pass
   mov al, 1
test_pass:
   ret
main ENDP
	
end

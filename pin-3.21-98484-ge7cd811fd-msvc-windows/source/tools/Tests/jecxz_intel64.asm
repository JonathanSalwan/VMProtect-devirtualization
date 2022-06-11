;
; Copyright (C) 2014-2014 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC main

; This assembly file should built to an executable file
; It tests the correctness of jecxz instruction and exits
; with status 0 if everything's OK.

.code

main PROC
   mov rcx, 100000000H
   xor rax,rax
   jecxz test_pass
   mov al, 1
test_pass:
   ret
main ENDP
	
end

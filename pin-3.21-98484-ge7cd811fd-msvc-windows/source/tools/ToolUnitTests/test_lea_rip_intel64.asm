;
; Copyright (C) 2011-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

.code

VerifyLeaRip PROC
    
	push rbx
    ;cannot write rbx, [rip]  in masm
    DB 48h, 8Dh, 1Dh, 00h, 00h, 00h, 00h
	call VerifyLeaLab1
VerifyLeaLab1:
    pop rdx
	xor rax,rax
	add rbx, 5
	cmp rdx, rbx
	je VerifyLeaLabNextTest
	pop rbx
    ret
VerifyLeaLabNextTest:
     ;cannot write rbx, [rip+5]  in masm
    DB 48h, 8Dh, 1Dh, 05h, 00h, 00h, 00h
	call VerifyLeaLab2
VerifyLeaLab2:
    pop rdx
	cmp rdx, rbx
	je VerifyLeaLabPassed
	pop rbx
    ret
VerifyLeaLabPassed:
    inc rax
    pop rbx
    ret
VerifyLeaRip ENDP


end
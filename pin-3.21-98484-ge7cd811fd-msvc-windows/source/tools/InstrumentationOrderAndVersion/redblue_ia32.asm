;
; Copyright (C) 2012-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC maina
PUBLIC red
PUBLIC blue
PUBLIC common
.686
.model flat, c
.code
maina PROC	
	mov eax,0
	cmp eax,0
    call red
	call blue
	mov eax,1
	cmp eax,0
	call red
	call blue
	ret
maina ENDP
	

red PROC		
	jz r2
	jmp common
r2:
	jmp common
red ENDP
	

blue PROC		
	jz b2
	jmp common
b2:	
	jmp common
blue ENDP
	

common PROC
	ret
common ENDP
						
end
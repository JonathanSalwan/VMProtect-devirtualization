;
; Copyright (C) 2010-2010 Intel Corporation.
; SPDX-License-Identifier: MIT
;

.code
GetMxcsr PROC
	stmxcsr DWORD PTR [rcx]
	ret

GetMxcsr ENDP

SetMxcsr PROC
	ldmxcsr DWORD PTR [rcx]
	ret

SetMxcsr ENDP

end
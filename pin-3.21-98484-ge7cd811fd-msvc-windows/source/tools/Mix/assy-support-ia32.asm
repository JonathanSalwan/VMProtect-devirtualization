;
; Copyright (C) 2008-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

.686
.xmm
.model flat,c

	.CODE
	ALIGN 4
	mix_fp_save PROC
	mov ecx, DWORD PTR[esp+4]
	fxsave  BYTE PTR [ecx]
	emms
	ret 
	mix_fp_save ENDP

	.CODE
	ALIGN 4
	mix_fp_restore PROC
	mov ecx, DWORD PTR[esp+4]
	fxrstor  BYTE PTR [ecx]
	ret 
	mix_fp_restore ENDP

	END

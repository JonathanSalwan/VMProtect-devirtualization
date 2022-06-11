/*
 * Copyright (C) 2008-2008 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.intel_syntax noprefix	
.globl main
main:	
	call l
	addr32 mov rax,[eax+8]
	mov rax,0
	ret
l:
	mov rax, [rsp]
	ret
	

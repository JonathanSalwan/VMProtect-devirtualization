/*
 * Copyright (C) 2008-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

	.text
#if defined(TARGET_MAC)
.globl _DoMath
_DoMath:
#else
.globl DoMath
DoMath:
#endif
    /*
     * Push "0.0" on the stack, then "1.0".
     */
    finit
    fldz
    fld1

    /*
     * This raises a SEGV.
     */
    mov     $0, %ecx
    mov     (%ecx), %ecx

    /* does not return */
	ret

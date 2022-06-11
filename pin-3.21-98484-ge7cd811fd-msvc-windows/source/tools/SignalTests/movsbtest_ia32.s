/*
 * Copyright (C) 2006-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * void CopyWithMovsb(void *src, void *dst, size_t size)
 */
.text
    .align 4
#if defined(TARGET_MAC)
.globl _CopyWithMovsb
_CopyWithMovsb:
#else
.globl CopyWithMovsb
CopyWithMovsb:
#endif
    movl    4(%esp), %esi   /* src */
    movl    8(%esp), %edi   /* dst */
    movl    12(%esp), %ecx  /* size */
    rep movsb
    ret

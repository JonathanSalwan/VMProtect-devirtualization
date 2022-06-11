/*
 * Copyright (C) 2005-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * CopyWithXmm(char *dest, const char *src, size_t size)
 *
 *  Copy a memory block using XMM registers.  Assumes that
 *  'dest' and 'src' are 16-byte aligned and 'size' is a
 *  multiple of 16 bytes.
 */
.text
	.align 4
#if defined(TARGET_MAC)
.globl _CopyWithXmm
_CopyWithXmm:
#else
.globl CopyWithXmm
CopyWithXmm:
#endif
    shr     $0x5, %rdx
.L1:
    movdqa  (%rsi), %xmm0
    movdqa  0x10(%rsi), %xmm1
    movdqa  %xmm0, (%rdi)
    movdqa  %xmm1, 0x10(%rdi)
    lea     0x20(%rsi), %rsi
    lea     0x20(%rdi), %rdi
    dec     %rdx
    jne     .L1
	ret


/*
 * CopyWithXmmDelay(char *dest, const char *src, size_t size)
 *
 *  This is exactly the same as CopyWithXmm() except that has
 *  an internal delay that makes it more likely for a signal
 *  to interrupt the copy during a critical point.
 */
.text
	.align 4
#if defined(TARGET_MAC)
.globl _CopyWithXmmDelay
_CopyWithXmmDelay:
#else
.globl CopyWithXmmDelay
CopyWithXmmDelay:
#endif
    shr     $0x5, %rdx
.L2:
    movdqa  (%rsi), %xmm0
    movdqa  0x10(%rsi), %xmm1

    /*
     * Delay for a while.  The values in the XMM registers are live
     * at this point, so this delay loop opens up a timing window that
     * makes it more likely for a signal to interrupt the copy when
     * the XMM registers are being used.
     */
    mov     $0x1000, %rax
.L3:
    dec     %rax
    jne     .L3

    movdqa  %xmm0, (%rdi)
    movdqa  %xmm1, 0x10(%rdi)
    lea     0x20(%rsi), %rsi
    lea     0x20(%rdi), %rdi
    dec     %rdx
    jne     .L2
	ret


/*
 * CopyWithXmmFault(char *dest, const char *src, size_t size)
 *
 *  This is exactly the same as CopyWithXmm() except that there is
 *  an illegal memory access to address 0x0 at a critical point.
 *  The caller is expected to set up a SIGSEGV handler to catch the
 *  illegal access.  The handler must change the value of %rax to
 *  a valid memory location.
 */
.text
	.align 4
#if defined(TARGET_MAC)
.globl _CopyWithXmmFault
_CopyWithXmmFault:
#else
.globl CopyWithXmmFault
CopyWithXmmFault:
#endif
    shr     $0x5, %rdx
.L4:
    movdqa  (%rsi), %xmm0
    movdqa  0x10(%rsi), %xmm1

    /*
     * This causes a SIGSEGV.  The caller must fix %rax in a handler.
     */
    mov     $0x0, %rax
    mov     (%rax), %rax

    movdqa  %xmm0, (%rdi)
    movdqa  %xmm1, 0x10(%rdi)
    lea     0x20(%rsi), %rsi
    lea     0x20(%rdi), %rdi
    dec     %rdx
    jne     .L4
	ret

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
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx

	movl	8(%ebp), %esi
	movl	12(%ebp), %ebx
	movl	16(%ebp), %ecx
    shr     $0x5, %ecx

.L1:
    movdqa  (%ebx), %xmm0
    movdqa  0x10(%ebx), %xmm1
    movdqa  %xmm0, (%esi)
    movdqa  %xmm1, 0x10(%esi)
    lea     0x20(%ebx), %ebx
    lea     0x20(%esi), %esi
    dec     %ecx
    jne     .L1

	popl	%ebx
	popl	%esi
	popl	%ebp
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
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx

	movl	8(%ebp), %esi
	movl	12(%ebp), %ebx
	movl	16(%ebp), %ecx
    shr     $0x5, %ecx

.L2:
    movdqa  (%ebx), %xmm0
    movdqa  0x10(%ebx), %xmm1

    /*
     * Delay for a while.  The values in the XMM registers are live
     * at this point, so this delay loop opens up a timing window that
     * makes it more likely for a signal to interrupt the copy when
     * the XMM registers are being used.
     */
    mov     $0x1000, %eax
.L3:
    dec     %eax
    jne     .L3

    movdqa  %xmm0, (%esi)
    movdqa  %xmm1, 0x10(%esi)
    lea     0x20(%ebx), %ebx
    lea     0x20(%esi), %esi
    dec     %ecx
    jne     .L2

	popl	%ebx
	popl	%esi
	popl	%ebp
	ret


/*
 * CopyWithXmmFault(char *dest, const char *src, size_t size)
 *
 *  This is exactly the same as CopyWithXmm() except that there is
 *  an illegal memory access to address 0x0 at a critical point.
 *  The caller is expected to set up a SIGSEGV handler to catch the
 *  illegal access.  The handler must change the value of %eax to
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
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx

	movl	8(%ebp), %esi
	movl	12(%ebp), %ebx
	movl	16(%ebp), %ecx
    shr     $0x5, %ecx

.L4:
    movdqa  (%ebx), %xmm0
    movdqa  0x10(%ebx), %xmm1

    /*
     * This causes a SIGSEGV.  The caller must fix %eax in a handler.
     */
    movl    $0x0, %eax
    movl    (%eax), %eax

    movdqa  %xmm0, (%esi)
    movdqa  %xmm1, 0x10(%esi)
    lea     0x20(%ebx), %ebx
    lea     0x20(%esi), %esi
    dec     %ecx
    jne     .L4

	popl	%ebx
	popl	%esi
	popl	%ebp
	ret

/*
 * Copyright (C) 2008-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text
    .align 4
#if defined(TARGET_MAC)
.globl _DFSet
.globl _DidTest
.globl _Flags
#define DFSet _DFSet
#define DidTest _DidTest
#define Flags _Flags

.globl _SetAndClearDF
_SetAndClearDF:
#else
.globl SetAndClearDF
SetAndClearDF:
#endif
    std
    movl    $1, DFSet

    /*
     * Delay a little while to make it more likely that a signal
     * will arrive while DF is set.
     */
    mov     $0x1000, %eax
.L3:
    dec     %eax
    jne     .L3

    movl    $0, DFSet
    cld
    ret


    .align 4
#if defined(TARGET_MAC)
.globl _SignalHandler
_SignalHandler:
#else
.globl SignalHandler
SignalHandler:
#endif
    /*
     * Save the flags in 'Flags'.
     */
    pushf
    pop     %eax
    mov     %eax, Flags

    /*
     * The test is only interesting if the signal arrived while DF was
     * set (above in SetAndClearDF).
     */
    mov     DFSet, %eax
    mov     %eax, DidTest
    ret

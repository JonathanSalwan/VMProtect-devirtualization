/*
 * Copyright (C) 2010-2010 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

    .text
.globl One
    .type   One, @function
One:
    /*
     * We force the thread to get an ASYNC_BREAK on this instruction, which is the
     * target of an indirect jump.
     */
    jmp     *%eax

    /*
     * The tool should change the PC to Two() when the ASYNC_BREAK occurs, so the
     * rest of this function should be skipped.
     */
    movl    $1, %eax
    ret


.globl Two
    .type   Two, @function
Two:
    movl    $2, %eax
    ret


.globl GetValue
    .type   GetValue, @function
GetValue:
    mov     4(%esp), %eax
    call    *%eax
    ret


.globl Breakpoint
    .type   Breakpoint, @function
Breakpoint:
    mov     $0, %eax
.L1:
    add     $1, %eax
    cmp     $99999999, %eax
    jbe     .L1

    /*
     * The debugger places a breakpoint here, but the tool intercepts the BREAKPOINT event.
     * If the breakpoint triggers before the other thread enters the One() function, the tool
     * squashes the breakpoint and moves the PC back to the start of Breakpoint().  The delay
     * loop waits a while, and then the breakpoint re-triggers.  This repeats until the other
     * thread is at the first instruction in One(), so the breakpoint is guaranteed to trigger
     * when the other thread is at that instruction.
     */
.globl BreakpointLocation
    .type   BreakpointLocation, @function
BreakpointLocation:
    nop

    ret

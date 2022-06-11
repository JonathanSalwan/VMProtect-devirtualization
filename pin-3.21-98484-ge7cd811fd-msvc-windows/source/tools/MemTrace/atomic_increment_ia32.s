/*
 * Copyright (C) 2011-2011 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.global numthreadsStarted
.type AtomicIncrement, @function
.global AtomicIncrement
AtomicIncrement:
    lea     numthreadsStarted, %ecx
    incl     (%ecx)
    ret



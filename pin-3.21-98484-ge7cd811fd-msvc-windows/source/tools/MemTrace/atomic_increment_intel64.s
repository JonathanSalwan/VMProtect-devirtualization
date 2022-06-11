/*
 * Copyright (C) 2011-2014 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.global numthreadsStarted
.type AtomicIncrement, @function
.global AtomicIncrement
AtomicIncrement:
    lea     numthreadsStarted(%rip), %rcx
    incl     (%rcx)
    ret




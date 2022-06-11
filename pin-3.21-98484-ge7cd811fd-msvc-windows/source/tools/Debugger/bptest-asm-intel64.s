/*
 * Copyright (C) 2008-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

    .text
.globl foo
foo:
    push    %rbp        /* BP here and ... */
    mov     %rsp, %rbp  /* ... here, to test for BP's on adjacent instructions */
    sub     $8, %rsp
    call    sub
    call    sub
    leave               /* BP here to test BP on target of indirect jump */
    ret

.globl sub
sub:
    ret                 /* BP here to test BP on indirect jump */

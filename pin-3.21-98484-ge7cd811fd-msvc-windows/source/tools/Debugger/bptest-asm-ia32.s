/*
 * Copyright (C) 2008-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

    .text
.globl foo
foo:
    pushl   %ebp        /* BP here and ... */
    movl    %esp, %ebp  /* ... here, to test for BP's on adjacent instructions */
    subl    $8, %esp
    call    sub
    call    sub
    leave               /* BP here to test BP on target of indirect jump */
    ret

.globl sub
sub:
    ret                 /* BP here to test BP on indirect jump */

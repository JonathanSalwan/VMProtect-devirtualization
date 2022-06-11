/*
 * Copyright (C) 2007-2007 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

# this code pattern tests an ip-relative displacement
# on a call instruction in the probe area.
	
    .text
.globl Bar
    .type   Bar, @function
Bar:
    pushq   %rbp
    movq    %rsp, %rbp
    leave
    ret

.globl pf
    .text
    .align 8
    .type   pf, @object
    .size   pf, 8
pf:
    .quad   Bar

    .text
.globl Foo
    .type   Foo, @function
Foo:
    pushq   %rbp
    movq    %rsp, %rbp

    call    *(pf-.-6)(%rip)  # 6 = size of this call instruction

    leave
    ret
	
# this code pattern tests an ip-relative jmp in the probe area.
	
.globl pt
    .text
    .align 8
    .type   pt, @object
    .size   pt, 8
pt:
    .quad   Bar

    .text
.globl Haha
    .type   Haha, @function
Haha:
    jmp    *(pt-.-6)(%rip)  # 6 = size of this instruction

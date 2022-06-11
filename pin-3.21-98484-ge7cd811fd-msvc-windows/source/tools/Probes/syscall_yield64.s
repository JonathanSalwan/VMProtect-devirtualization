/*
 * Copyright (C) 2009-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

# this routine calls a system call in the first bytes of the function
# within the probe space.
#
.text
.global my_yield
.type my_yield, function

my_yield:
    mov    $0x18,%eax   # this is the number of SYS_sched_yield
    syscall
    cmp    $0xfffffffffffff001,%rax
    jae    .fail
    retq
.fail:
    or     $0xffffffffffffffff,%rax
    retq


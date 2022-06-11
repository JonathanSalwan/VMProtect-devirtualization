/*
 * Copyright (C) 2011-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

    .text
.globl BareExitGroup
BareExitGroup:
    movq    $0,%rdi     # first argument: exit code
#if defined(TARGET_MAC)
    movq    $0x2000001,%rax    # system call number (exit) - exit process
#elif defined(TARGET_LINUX)
    movq    $231,%rax   # system call number (sys_exit_group) - exit process
#endif

.globl BareExitTrap
BareExitTrap:
    syscall             # call kernel

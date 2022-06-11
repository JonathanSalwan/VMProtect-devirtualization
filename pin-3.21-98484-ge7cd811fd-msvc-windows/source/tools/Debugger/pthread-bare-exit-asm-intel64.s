/*
 * Copyright (C) 2011-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

    .text
.globl BareExit
BareExit:
    movq    $0,%rdi     # first argument: exit code
#if defined(TARGET_MAC)
    movq    $0x2000169,%rax    # system call number (bthread_terminate) - exit thread
#elif defined(TARGET_LINUX)
    movq    $60,%rax    # system call number (sys_exit) - exit thread
#endif


.globl BareExitTrap
BareExitTrap:
    syscall             # call kernel

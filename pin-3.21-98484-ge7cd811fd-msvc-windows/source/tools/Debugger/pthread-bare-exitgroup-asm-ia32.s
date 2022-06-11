/*
 * Copyright (C) 2011-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

    .text
.globl BareExitGroup
BareExitGroup:
#if defined(TARGET_MAC)
    sub     $8, %esp	# On function entry stack is aligned to 12%16 (0x...C) since return IP was pushed to stack, 
    					# moving down to 4%12
    push    $0     # first argument: exit code, stack is aligned to 16 bits
    sub     $4, %esp	# This mimics what call instruction does (push return IP), This is what syscall expect.
    movl    $1,%eax     # system call number (exit) - exit process
#elif defined(TARGET_LINUX)
    movl    $0,%ebx     # first argument: exit code
    movl    $252,%eax   # system call number (sys_exit_group) - exit process
#endif

.globl BareExitTrap
BareExitTrap:
    int     $0x80       # call kernel

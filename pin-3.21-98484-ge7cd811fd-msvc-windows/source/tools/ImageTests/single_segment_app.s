/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>
#include <asm/unistd.h>

DECLARE_FUNCTION(_start)

NAME(_start):
    BEGIN_STACK_FRAME
    add $100, STACK_PTR
    PREPARE_UNIX_SYSCALL($__NR_exit_group)
    mov $0, SYSCALL_PARAM1
    INVOKE_SYSCALL
    END_STACK_FRAME
    ud2

.section .data, "aw"
.byte 1

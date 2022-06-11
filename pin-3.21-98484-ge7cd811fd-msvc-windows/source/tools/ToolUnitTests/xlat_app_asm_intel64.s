/*
 * Copyright (C) 2020-2020 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  The function executes XLAT on mapped memory, both with and without address size prefix.
 *  The memory is expected to be in the lower 2 GB of the process address space.
 */
.global test_xlat
.type test_xlat, @function
test_xlat:
           push %rbp
           mov %rsp, %rbp
           mov %rdi, %rbx   # the input buffer is in rdi
           push %rax
           mov $2, %rax     # rax gets a small value that fits into AL
           xlat             # XLAT without prefix
           mov $2, %rax
    addr32 xlat             # XLAT with 0x67 prefix
           pop %rax
           leave
           ret

/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <syscall.h>
#include <unistd.h>
#include <errno.h>

/*! @file
 *
 * This test checks that the emulation of arch_prctl calls the
 * native syscall when it is not a known sub-function
 */

int main()
{
    const int func = 0x3000;
    long arg       = 0;
    void* ptr      = &arg;
    int res        = syscall(__NR_arch_prctl, func, ptr);

    printf("Syscall return: %d\n", res);

    if (res != -1) return 1;

    return 0;
}

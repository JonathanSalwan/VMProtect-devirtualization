/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>

unsigned int sleep(unsigned int seconds)
{
    fprintf(stderr, "ERROR: In deepbind_syscalls.c implementation of sleep\n");
    exit(3);
}

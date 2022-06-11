/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sched.h>
#include <unistd.h>

int main()
{
    sched_yield();
    sleep(1);
    return 0;
}

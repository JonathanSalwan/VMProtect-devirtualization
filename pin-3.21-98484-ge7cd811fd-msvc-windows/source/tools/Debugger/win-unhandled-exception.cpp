/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

int main()
{
    volatile int zero = 0;
    volatile int res  = 10 / zero;
    return 0;
}

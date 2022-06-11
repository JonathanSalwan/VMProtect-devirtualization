/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

extern unsigned int bar();

int main()
{
    if (bar() == 0x1ee7) return 0;
    return 1;
}

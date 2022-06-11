/*
 * Copyright (C) 2008-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

extern void DoXmm();
extern void ZeroXmms();

int main()
{
    ZeroXmms();
    DoXmm();
    return 0;
}

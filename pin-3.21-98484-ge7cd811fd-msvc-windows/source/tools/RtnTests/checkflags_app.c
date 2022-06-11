/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

extern int CheckFlags(unsigned int*, unsigned int*);

int main()
{
    unsigned int before = 0;
    unsigned int after  = 0;
    if (0 != CheckFlags(&before, &after))
    {
        fprintf(stderr, "APP ERROR: Flags register was corrupted: before 0x%x, after 0x%x\n", before, after);
        return 1;
    }
    return 0;
}

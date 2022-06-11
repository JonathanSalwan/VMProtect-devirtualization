/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>

void* getFlags(void*);

int main()
{
    union
    {
        unsigned long long ull;
        void* voidp;
    } flags;

    memset(&flags, 0, sizeof(flags));

    // Call getFlags which in turn sets the flags register to 7,
    // call modifyFlags(), and return the value of the flags register
    // returned from modifyFlags()
    flags.voidp = getFlags((void*)7);
    printf("Flags: %llx\n", flags.ull);

    return 0;
}

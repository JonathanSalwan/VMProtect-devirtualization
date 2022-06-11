/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

extern "C" int VerifyLeaRip();

int main()
{
    // verify that the instruction lea reg, [rip+offset] is translated correctly
    if (!VerifyLeaRip())
    {
        fprintf(stderr, "VerifyLeaRip failed\n");
        return (-1);
    }
    return (0);
}

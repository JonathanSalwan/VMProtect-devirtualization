/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

extern "C" int SegAccessRtn(int val);
extern "C" int SegAccessStrRtn(int val);
int main()
{
    int value;
    if ((SegAccessRtn(5) != 105) || (SegAccessRtn(6) != 106))
    {
        fprintf(stderr, "SegAccessRtn failed\n");
        return -1;
    }
    printf("SegAccessRtn success\n");

    value = SegAccessStrRtn(30);
    if (value != 30)
    {
        fprintf(stderr, "SegAccessStrRtn failed (%d, not 30)\n", value);
        return -1;
    }
    printf("SegAccessStrRtn success\n");

    return 0;
}

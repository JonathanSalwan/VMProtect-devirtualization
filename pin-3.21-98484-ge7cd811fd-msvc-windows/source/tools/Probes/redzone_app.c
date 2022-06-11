/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

extern int CheckRedZone(void);

int main()
{
    int checkRedZone = CheckRedZone();
    if (checkRedZone != 0)
    {
        printf("Detected redzone overrun at byte %d\n", checkRedZone);
        return 1;
    }
    return 0;
}

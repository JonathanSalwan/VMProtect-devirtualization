/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>

extern int my_yield();

int my_marker() { return 1; }

int main()
{
    int i;

    printf("Calling my_yield()\n");

    for (i = 0; i < 100; i++)
    {
        int res = my_yield();
        if (res != 0)
        {
            printf("my_yield failed...\n");
            exit(1);
        }
    }

    my_marker();

    return (0);
}

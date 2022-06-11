/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>

void bye1() { printf("That was all, folks - 1\n"); }
void bye2() { printf("That was all, folks - 2\n"); }

int main()
{
    int res = atexit(bye1);
    if (res != 0)
    {
        fprintf(stderr, "cannot set exit function\n");
        return EXIT_FAILURE;
    }
    res = atexit(bye2);
    if (res != 0)
    {
        fprintf(stderr, "cannot set exit function\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

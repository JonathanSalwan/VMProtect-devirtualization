/*
 * Copyright (C) 2019-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: <arg> \"\" <arg2>\n");
        return 1;
    }

    if (strlen(argv[1]) == 0 || strlen(argv[2]) != 0 || strlen(argv[3]) == 0)
    {
        fprintf(stderr, "Usage: <arg> \"\" <arg2>");
        return 1;
    }
    printf("ok\n");
    return 0;
}

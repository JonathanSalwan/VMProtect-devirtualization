/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

int main(int argc, char** argv)
{
    char* filename;

    if (argc > 1)
        filename = argv[1];
    else
        filename = "foobar.out";

    FILE* fp = fopen(filename, "w");

    fprintf(fp, "Hello, World\n");
    fclose(fp);
    return 0;
}

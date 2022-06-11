/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * this application calls a user-written assembly routine which contains
 * a specific code pattern.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Haha();

int main(int argc, char* argv[])
{
    char* buffer;

    buffer = (char*)malloc(64);
    strcpy(buffer, "abc");
    printf("%s\n", buffer);

    Haha();
    printf("returned from haha & bar.\n");

    free(buffer);
    return 0;
}

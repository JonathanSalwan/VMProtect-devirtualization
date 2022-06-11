/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * this application calls a user-written version of free which contains
 * a specific code pattern.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void do_nothing();

int main(int argc, char* argv[])
{
    char* buffer;

    buffer = (char*)malloc(64);
    strcpy(buffer, "abc");
    printf("%s\n", buffer);
    do_nothing();
    printf("returned from do_nothing.\n");
    free(buffer);
    return 0;
}

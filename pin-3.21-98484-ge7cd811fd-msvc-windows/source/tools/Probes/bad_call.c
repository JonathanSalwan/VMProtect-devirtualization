/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * this application calls a user-written function that contains a bad code pattern.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void bad_call();

int main(int argc, char* argv[])
{
    char* buffer;

    buffer = (char*)malloc(64);
    strcpy(buffer, "abc");
    printf("%s\n", buffer);
    bad_call();
    printf("returned from bad_call.\n");
    free(buffer);
    return 0;
}

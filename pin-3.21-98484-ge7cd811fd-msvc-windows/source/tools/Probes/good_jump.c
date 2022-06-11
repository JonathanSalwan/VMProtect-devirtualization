/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * this application calls a user-written function that contains a good code pattern.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void good_jump();

int main(int argc, char* argv[])
{
    char* buffer;

    buffer = (char*)malloc(64);
    strcpy(buffer, "abc");
    printf("%s\n", buffer);
    good_jump();
    printf("returned from good_jump.\n");
    free(buffer);
    return 0;
}

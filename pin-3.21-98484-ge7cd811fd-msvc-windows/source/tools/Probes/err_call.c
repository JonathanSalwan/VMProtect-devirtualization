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

int main(int argc, char* argv[])
{
    char* buffer;

    buffer = (char*)malloc(64);
    strcpy(buffer, "abc");
    printf("%s\n", buffer);
    call_function();
    printf("returned from call_function & do_nothing.\n");
    free(buffer);
    return 0;
}

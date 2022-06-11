/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * this application calls malloc and prints the result.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void* my_malloc(size_t size) { return malloc(size); }

int main(int argc, char* argv[])
{
    char* buffer1;
    char* buffer2;
    char* buffer3;
    char* buffer4;

    buffer1 = (char*)my_malloc(64);
    printf("little_malloc: 0x%lx\n", buffer1);

    buffer2 = (char*)my_malloc(128);
    printf("little_malloc: 0x%lx\n", buffer2);

    buffer3 = (char*)my_malloc(256);
    printf("little_malloc: 0x%lx\n", buffer3);

    buffer4 = (char*)my_malloc(512);
    printf("little_malloc: 0x%lx\n", buffer4);

    printf("little_malloc: freeing 0x%lx\n", buffer1);
    free(buffer1);
    printf("little_malloc: freeing 0x%lx\n", buffer2);
    free(buffer2);
    printf("little_malloc: freeing 0x%lx\n", buffer3);
    free(buffer3);
    printf("little_malloc: freeing 0x%lx\n", buffer4);
    free(buffer4);

    printf(" Test passed.\n");

    return 0;
}

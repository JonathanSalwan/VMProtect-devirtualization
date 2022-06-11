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

static void* my_malloc(size_t size)
{
    void* ptr = malloc(size);
    return ptr;
}

static void my_free(void* ptr) { free(ptr); }

int main(int argc, char* argv[])
{
    char* buffer1 = NULL;
    char* buffer2 = NULL;
    char* buffer3 = NULL;
    char* buffer4 = NULL;
    int success   = 0;

    buffer1 = (char*)my_malloc(64);
    printf("little_malloc: %p\n", (void*)buffer1);

    buffer2 = (char*)my_malloc(128);
    printf("little_malloc: %p\n", (void*)buffer2);

    buffer3 = (char*)my_malloc(256);
    printf("little_malloc: %p\n", (void*)buffer3);

    buffer4 = (char*)my_malloc(512);
    printf("little_malloc: %p\n", (void*)buffer4);

    if (buffer1 && buffer2 && buffer3 && buffer4) success = 1;

    if (success)
    {
        printf(" Test passed.\n");

        my_free(buffer1);
        my_free(buffer2);
        my_free(buffer3);
        my_free(buffer4);
    }
    else
        printf(" Test failed.\n");

    return 0;
}

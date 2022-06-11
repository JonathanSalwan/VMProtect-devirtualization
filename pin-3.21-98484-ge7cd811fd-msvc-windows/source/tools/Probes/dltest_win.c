/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

void Load(char* name, int expect)
{
    int val;

    HMODULE handle;
    int (*sym)();

    handle = LoadLibrary(name);
    if (handle == 0)
    {
        fprintf(stderr, "Load of %s failed\n", name);
        fflush(stderr);
        exit(1);
    }

    sym = (int (*)())GetProcAddress(handle, "one");
    fprintf(stderr, "Address of sym is %p\n", sym);
    fflush(stderr);

    if (sym == 0)
    {
        fprintf(stderr, "GetProcAddress() of %s failed\n", name);
        fflush(stderr);
        exit(1);
    }

    val = sym();
    if (val != expect)
    {
        fprintf(stderr, "Error: Bad value returned.\n");
        fflush(stderr);
        exit(1);
    }
}

int main()
{
    int* ptr;
    size_t size = 2048;

    ptr = malloc(size);

    if (ptr > 0x0)
    {
        fprintf(stderr, "Allocation of %d was successful.\n", size);
        fflush(stderr);
    }

    Load("one.dll", 1);
    Load("two.dll", 2);

    if (ptr) free(ptr);

    return 0;
}

/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <stdio.h>

#ifdef TARGET_MAC
#define LIBCLOSE "libclose1.dylib"
#else
#define LIBCLOSE "libclose1.so"
#endif

void Load(const char* name)
{
    void* handle;

    handle = dlopen(name, RTLD_LAZY);
    if (handle == 0)
    {
        fprintf(stderr, "Load of %s failed\n", name);
        exit(1);
    }

    dlclose(handle);
}

int main()
{
    Load(LIBCLOSE);

    return 0;
}

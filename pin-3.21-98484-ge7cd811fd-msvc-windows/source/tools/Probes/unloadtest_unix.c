/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <stdio.h>
#include "tool_macros.h"

void Load(char* name, int expect)
{
    int val;

    void* handle;
    int (*sym)();

    handle = dlopen(name, RTLD_LAZY);
    if (handle == 0)
    {
        fprintf(stderr, "Load of %s failed\n", name);
        exit(1);
    }

    sym = (int (*)())dlsym(handle, "one");
    fprintf(stderr, "Address of sym is %p\n", sym);

    if (sym == 0)
    {
        fprintf(stderr, "Dlsym of %s failed\n", name);
        exit(1);
    }

    val = sym();
    if (val != expect) exit(1);

    dlclose(handle);
}

int main()
{
    Load(SHARED_LIB("libone"), 1);
    Load(SHARED_LIB("libtwo"), 2);

    return 0;
}

/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <stdio.h>

#ifdef TARGET_MAC
#define ONE "libone.dylib"
#define TWO "libtwo.dylib"
#else
#define ONE "libone.so"
#define TWO "libtwo.so"
#endif

void Load(char* name, int expect)
{
    int val;
    double dval;

    void* handle;
    int (*sym)();
    double (*fsin)(double);

    handle = dlopen(name, RTLD_LAZY);
    if (handle == 0)
    {
        fprintf(stderr, "Load of %s failed\n", name);
        exit(1);
    }

    sym = (int (*)())dlsym(handle, "one");
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
    int i;
    for (i = 0; i < 100; i++)
    {
        switch (1 + rand() % 2)
        {
            case 1:
                Load(ONE, 1);
                break;
            case 2:
                Load(TWO, 2);
                break;
        }
    }

    return 0;
}

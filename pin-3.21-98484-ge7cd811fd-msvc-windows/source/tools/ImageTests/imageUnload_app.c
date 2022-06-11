/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>

void Open(char* filename)
{
    void* dlh = dlopen(filename, RTLD_LAZY);
    if (!dlh)
    {
        fprintf(stderr, " Failed to load: %s because: %s", filename, dlerror());
        exit(2);
    }
    dlclose(dlh);
}

int main(int argc, char** argv)
{
    if (argc < 1)
    {
        fprintf(stderr, "No image name to load has been supplied");
        fflush(stderr);
        return 1;
    }

    Open(argv[1]);
    return 0;
}

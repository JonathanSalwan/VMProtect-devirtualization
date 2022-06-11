/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <dlfcn.h>

enum ExitType
{
    RES_SUCCESS = 0,     // 0
    RES_LOAD_FAILED,     // 1
    RES_RES_INVALID_ARGS // 2
};

/*
    Expected argv arguments:
    [1] image to load
*/
int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "No enough arguments\n");
        fflush(stderr);
        return RES_RES_INVALID_ARGS;
    }

    void* handle;

    handle = dlopen(argv[1], RTLD_LAZY);
    if (!handle)
    {
        fprintf(stderr, " Failed to load: %s because: %s\n", argv[1], dlerror());
        fflush(stderr);
        return RES_LOAD_FAILED;
    }

    return RES_SUCCESS;
}

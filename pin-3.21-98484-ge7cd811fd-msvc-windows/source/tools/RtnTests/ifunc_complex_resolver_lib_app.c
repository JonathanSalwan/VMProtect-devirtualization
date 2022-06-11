/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#ifdef TARGET_IA32
#define ARCH_DIR "ia32"
#elif TARGET_IA32E
#define ARCH_DIR "intel64"
#else
#error Unsupported architecture
#endif

const char* ifunc() __attribute__((ifunc("ifunc_resolver")));

const char* ifunc_not_exist() { return "File doesn't exist"; }

const char* ifunc_exist() { return "File exist"; }

void* ifunc_resolver()
{
    const char* fname = "obj-" ARCH_DIR "/ifunc_complex_resolver.file";
    errno             = ENOENT; // Access the TLS
    if (NULL != fname && 0 == access(fname, F_OK))
    {
        return (void*)ifunc_exist;
    }
    return (void*)ifunc_not_exist;
}

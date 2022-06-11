/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <dlfcn.h>
#include <limits.h>
#include <link.h>
using std::pair;
using std::vector;

// The executable filename of this program
char progname[4096];

/*
 * Iterate over all images known to the loader and print their memory regions
 */
int dl_iterate(struct dl_phdr_info* info, size_t size, void* data)
{
    const char* realname = info->dlpi_name;
    vector< pair< void*, void* > > vecSegments;
    if (strstr(realname, "linux-gate.so") == realname || strstr(realname, "linux-vdso.so") == realname)
    {
        // Don't count VDSO, PIN doesn't repont it intentionally
        return 0;
    }
    for (int j = 0; j < info->dlpi_phnum; j++)
    {
        if (info->dlpi_phdr[j].p_type == PT_LOAD)
        {
            void* start = (void*)(info->dlpi_addr + info->dlpi_phdr[j].p_vaddr);
            void* end   = (void*)((char*)start + info->dlpi_phdr[j].p_memsz - 1);
            vecSegments.push_back(pair< void*, void* >(start, end));
            if (*realname == 0 && (void*)dl_iterate >= start && (void*)dl_iterate < end)
            {
                realname = progname;
            }
        }
    }
    if (*realname != 0)
    {
        for (vector< pair< void*, void* > >::iterator it = vecSegments.begin(); it != vecSegments.end(); it++)
        {
            printf("%s, %p-%p\n", realname, it->first, it->second);
        }
    }
    return 0;
}

int main(int argc, const char* argv[], char** envp)
{
    if (argc != 2)
    {
        printf("Usage: %s <path to lib>\n", argv[0]);
        return 1;
    }
    realpath(argv[0], progname);
    void* hDll = dlopen(argv[1], RTLD_LAZY | RTLD_GLOBAL);
    if (NULL == hDll)
    {
        printf("Failed to open %s - %s\n", argv[1], dlerror());
        return 1;
    }
    int (*return2)() = (int (*)())dlsym(hDll, "return2");
    if (NULL == return2)
    {
        printf("Failed to locate 'return2' - %s\n", dlerror());
        return 1;
    }

    if (return2() != 2)
    {
        printf("Bad value returned from 'return2'\n");
        return 1;
    }
    dl_iterate_phdr(dl_iterate, NULL);
    return 0;
}

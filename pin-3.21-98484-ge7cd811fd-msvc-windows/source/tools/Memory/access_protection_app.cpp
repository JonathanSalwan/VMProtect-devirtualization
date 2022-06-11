/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <stdio.h>
#if defined(TARGET_WINDOWS)
#include <cstdio>
#include <cstdlib>
#include <windows.h>
#define EXPORT_SYM extern "C" __declspec(dllexport) __declspec(noinline)
#else
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#define EXPORT_SYM extern "C"
#endif

enum ExitType
{
    RES_SUCCESS = 0,      // 0
    RES_INVALID_ARGS,     // 1
    RES_INVALID_TEST_TYPE // 2
};

EXPORT_SYM void NotifyPinAfterMmap(const char* addr) { fprintf(stderr, "Allocated %p\n", addr); }

#if defined(TARGET_WINDOWS)

size_t GetPageSize()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return static_cast< size_t >(sysInfo.dwPageSize);
}

const char* MmapWrapperNoMemoryAccess()
{
    return reinterpret_cast< const char* >(VirtualAlloc(0, GetPageSize(), MEM_COMMIT, PAGE_NOACCESS));
}

const char* MmapWrapperWriteAndReadMemoryAccess()
{
    return reinterpret_cast< const char* >(VirtualAlloc(0, GetPageSize(), MEM_COMMIT, PAGE_READWRITE));
}

#else

size_t GetPageSize() { return static_cast< size_t >(getpagesize()); }

const char* MmapWrapperNoMemoryAccess()
{
#if defined(TARGET_MAC)
    return reinterpret_cast< const char* >(mmap(0, GetPageSize(), PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0));
#else
    return reinterpret_cast< const char* >(mmap(0, GetPageSize(), PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
#endif
}

const char* MmapWrapperWriteAndReadMemoryAccess()
{
#if defined(TARGET_MAC)
    return reinterpret_cast< const char* >(mmap(0, GetPageSize(), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0));
#else
    return reinterpret_cast< const char* >(mmap(0, GetPageSize(), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
#endif
}

#endif

/*
    Expected argv arguments:
    [1] TestType
        1- mmap function is called by the application.
        2- mmap function is called by the tool.
*/
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
#if defined(TARGET_WINDOWS)
        fprintf(stderr, "Invalid number of arguments<%d>, expecting only one\n", argc);
#else
        fprintf(stderr, "Invalid number of arguments %d, expecting only one\n", argc);
#endif
        fflush(stderr);
        exit(RES_INVALID_ARGS);
    }

    switch (atoi(argv[1]))
    {
        case 1:
        {
            const char* ans = MmapWrapperNoMemoryAccess();
            NotifyPinAfterMmap(ans);
            ans = MmapWrapperWriteAndReadMemoryAccess();
            NotifyPinAfterMmap(ans);
            break;
        }
        case 2:
        {
            // do nothing
            break;
        }
        default:
            return RES_INVALID_TEST_TYPE;
    }

    return RES_SUCCESS;
}

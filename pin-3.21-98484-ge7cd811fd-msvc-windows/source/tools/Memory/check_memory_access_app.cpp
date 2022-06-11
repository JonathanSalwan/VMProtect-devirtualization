/*
 * Copyright (C) 2018-2021 Intel Corporation.
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

EXPORT_SYM void NotifyPinAfterMmap(const char* addr) { fprintf(stderr, "Allocated %p\n", addr); }

#if defined(TARGET_WINDOWS)

size_t GetPageSize()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return static_cast< size_t >(sysInfo.dwPageSize);
}

const char* allocatePage()
{
    DWORD oldProtect;
    const char* buffer = reinterpret_cast< const char* >(VirtualAlloc(0, GetPageSize() * 2, MEM_COMMIT, PAGE_READWRITE));
    VirtualProtect((LPVOID)(buffer + GetPageSize()), GetPageSize(), PAGE_NOACCESS, &oldProtect);
    return buffer;
}

#else

size_t GetPageSize() { return static_cast< size_t >(getpagesize()); }

const char* allocatePage()
{
#if defined(TARGET_MAC)
    const char* buffer =
        reinterpret_cast< const char* >(mmap(0, GetPageSize() * 2, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0));
#else
    const char* buffer =
        reinterpret_cast< const char* >(mmap(0, GetPageSize() * 2, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
#endif
    mprotect((void*)(buffer + GetPageSize()), GetPageSize(), PROT_NONE);
    return buffer;
}

#endif

int main(int argc, char* argv[])
{
    const char* ans = allocatePage();
    //Tool checks the last byte of the allocated page.
    NotifyPinAfterMmap(ans + GetPageSize() - 1);
    return 0;
}

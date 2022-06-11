/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <errno.h>
#include <syscall.h>
#include <unistd.h>
#include <asm/ldt.h>
#include <string.h>

struct FarPointer
{
    FarPointer(unsigned int v0, unsigned int v1) : _farPtr(v0), _segVal(v1) {}
    unsigned int _farPtr;
    unsigned int _segVal;
};

extern "C"
{
    int SetGs(const FarPointer* fp);
    int SetFs(const FarPointer* fp);

    unsigned int GetGsBase();
    unsigned int GetFsBase();
};

#define TLS_GET_GS_REG()                           \
    (                                              \
        {                                          \
            int __seg;                             \
            __asm("movw %%gs, %w0" : "=q"(__seg)); \
            __seg & 0xffff;                        \
        })

#define TLS_SET_GS_REG(val) __asm("movw %w0, %%gs" ::"q"(val))

#define TLS_GET_FS_REG()                           \
    (                                              \
        {                                          \
            int __seg;                             \
            __asm("movw %%fs, %w0" : "=q"(__seg)); \
            __seg & 0xffff;                        \
        })

#define TLS_SET_FS_REG(val) __asm("movw %w0, %%fs" ::"q"(val))

#define LDT_GET 0
#define LDT_SET 1

#ifndef __NR_set_thread_area
#define __NR_set_thread_area 243
#endif
#ifndef __NR_get_thread_area
#define __NR_get_thread_area 244
#endif
#ifndef SYS_set_thread_area
#define SYS_set_thread_area __NR_set_thread_area
#endif
#ifndef SYS_get_thread_area
#define SYS_get_thread_area __NR_get_thread_area
#endif

typedef struct
{
    unsigned int entry_number;
    unsigned int base_addr;
    unsigned int limit;
    unsigned int seg_32bit : 1;
    unsigned int contents : 2;
    unsigned int read_exec_only : 1;
    unsigned int limit_in_pages : 1;
    unsigned int seg_not_present : 1;
    unsigned int useable : 1;
} UserDesc;

int main()
{
    unsigned int currentGS = TLS_GET_GS_REG();
    unsigned int currentFS = TLS_GET_FS_REG();

    if ((currentGS == 0) || ((currentGS & 0x4) == 0x4))
    {
        unsigned int ldtEntry     = currentGS >> 3;
        unsigned int ldtNextEntry = ldtEntry + 1;

        UserDesc tableEntry;
        memset(&tableEntry, 0, sizeof(UserDesc));
        tableEntry.entry_number = ldtNextEntry;
        tableEntry.base_addr    = reinterpret_cast< unsigned int >(new unsigned int(0xabcd));
        tableEntry.limit        = 0x4000;

        int res = syscall(SYS_modify_ldt, LDT_SET, &tableEntry, sizeof(UserDesc));
        if (res < 0)
        {
            printf("LDT_SET for entry %d failed, code %d, %s\n", ldtNextEntry, errno, strerror(errno));
            return 0;
        }

        unsigned int newSegVal = (ldtNextEntry << 3) + 7;

        unsigned int GSRes = 0x1;
        unsigned int FSRes = 0x2;
        FarPointer farPtrGS(GSRes, newSegVal);
        FarPointer farPtrFS(FSRes, newSegVal);

        if ((SetGs(&farPtrGS) != GSRes) || (SetFs(&farPtrFS) != FSRes))
        {
            TLS_SET_GS_REG(currentGS);
            TLS_SET_FS_REG(currentFS);
            printf("LFS or LGS failed\n");
            return 0;
        }

        if ((TLS_GET_GS_REG() != newSegVal) || (TLS_GET_FS_REG() != newSegVal))
        {
            TLS_SET_GS_REG(currentGS);
            TLS_SET_FS_REG(currentFS);
            printf("GS or FS wrong value\n");
            return 0;
        }

        if ((GetGsBase() != 0xabcd) || (GetFsBase() != 0xabcd))
        {
            TLS_SET_GS_REG(currentGS);
            TLS_SET_FS_REG(currentFS);
            printf("GS or FS wrong base address\n");
            return 0;
        }
        TLS_SET_GS_REG(currentGS);
        TLS_SET_FS_REG(currentFS);
        printf("LFS and LGS passed successfully\n");
        printf("Base address of GS and FS was synchronuized successfully;\n");
    }
    else
    {
        unsigned int gdtEntry     = currentGS >> 3;
        unsigned int gdtNextEntry = gdtEntry + 1;

        UserDesc tableEntry;

        tableEntry.entry_number = gdtEntry;
        int res                 = syscall(SYS_get_thread_area, &tableEntry);
        if (res != 0)
        {
            printf("SYS_get_thread_area failed with error: %s\n", strerror(errno));
            return 0;
        }

        tableEntry.entry_number = gdtNextEntry;
        tableEntry.base_addr    = reinterpret_cast< unsigned int >(new unsigned int(0xabcd));

        res = syscall(SYS_set_thread_area, &tableEntry);
        if (res != 0)
        {
            TLS_SET_GS_REG(currentGS);
            TLS_SET_FS_REG(currentFS);
            printf("SYS_set_thread_area failed with error: %s\n", strerror(errno));
            return 0;
        }
        unsigned int newSegVal = (gdtNextEntry << 3) + 3;

        unsigned int GSRes = 0x1;
        unsigned int FSRes = 0x2;
        FarPointer farPtrGS(GSRes, newSegVal);
        FarPointer farPtrFS(FSRes, newSegVal);

        if ((SetGs(&farPtrGS) != GSRes) || (SetFs(&farPtrFS) != FSRes))
        {
            TLS_SET_GS_REG(currentGS);
            TLS_SET_FS_REG(currentFS);
            printf("LFS or LGS failed\n");
            return 0;
        }

        if ((TLS_GET_GS_REG() != newSegVal) || (TLS_GET_FS_REG() != newSegVal))
        {
            TLS_SET_GS_REG(currentGS);
            TLS_SET_FS_REG(currentFS);
            printf("GS or FS wrong value\n");
            return 0;
        }

        if ((GetGsBase() != 0xabcd) || (GetFsBase() != 0xabcd))
        {
            TLS_SET_GS_REG(currentGS);
            TLS_SET_FS_REG(currentFS);
            printf("GS or FS wrong base address\n");
            return 0;
        }

        TLS_SET_GS_REG(currentGS);
        TLS_SET_FS_REG(currentFS);
        printf("LFS and LGS passed successfully\n");
        printf("Base address of GS and FS was synchronized successfully;\n");
    }
    return 0;
}

/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <syscall.h>
#include <linux/unistd.h>
#include <asm/ldt.h>
#include <errno.h>
#include <string.h>
#include <sys/utsname.h>

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

typedef struct
{
    unsigned int d1;
    unsigned int d2;
} UserInfo;

int GetVal(unsigned int off)
{
    __asm("\
    push %edx;\
    mov 8(%ebp), %edx;\
    mov %fs:0x0(%edx), %edx;\
    mov %edx, %eax;\
    pop %edx");
}
const unsigned int value1 = 5;
const unsigned int value2 = 18;

#define GDT_NUM_OF_ENTRIES 3
#define GDT_ENTRIES 16

unsigned int GdtFirstEntry()
{
    static int first = 0;
    if (first) return first;

    UserDesc thrDescr;

    for (int i = 0; i < GDT_ENTRIES; i++)
    {
        thrDescr.entry_number = i;
        int res               = syscall(SYS_get_thread_area, &thrDescr);
        if ((res == 0) || (errno != EINVAL))
        {
            first = i;
            return first;
        }
    }
    fprintf(stderr, "First GDT entry is not found\n");
    exit(-1);
}

int main(int argc, char* argv[])
{
    int rc;
    UserDesc tr;
    int res;

    tr.entry_number = GdtFirstEntry();

    res = syscall(SYS_get_thread_area, &tr);
    if (res != 0)
    {
        printf("SYS_get_thread_area failed with error: %s\n", strerror(errno));
        return 0;
    }

    tr.entry_number                 = (unsigned)-1;
    tr.base_addr                    = (unsigned int)new UserInfo();
    ((UserInfo*)(tr.base_addr))->d1 = value1;
    ((UserInfo*)(tr.base_addr))->d2 = value2;

    res = syscall(SYS_set_thread_area, &tr);

    if (res != 0)
    {
        printf("SYS_set_thread_area failed with error: %s\n", strerror(errno));
        return 0;
    }

    TLS_SET_FS_REG((tr.entry_number << 3) + 3);

    if ((GetVal(0) == value1) && (GetVal(4) == value2))
    {
        printf("TEST PASSED\n");
    }
    else
    {
        printf("TEST FAILED\n");
    }
    return 0;
}

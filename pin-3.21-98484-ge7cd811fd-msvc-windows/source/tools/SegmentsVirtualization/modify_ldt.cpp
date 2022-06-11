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
#include <sys/types.h>

#define NTHREADS 4

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

#define LDT_GET 0
#define LDT_SET 1
#define FILL_ENTRIES 10

struct LDT_TABLE_ENTRY
{
    LDT_TABLE_ENTRY() : _val1(0), _val2(0) {}

    unsigned int _val1;
    unsigned int _val2;
};
inline unsigned int LDTEntryA(unsigned int base) { return base << 16; }

char buf[NTHREADS][3000];

void* thread_func(void* arg)
{
    unsigned long thread_no = (unsigned long)arg + 1;

    unsigned int gs_value = TLS_GET_GS_REG();
    sprintf(buf[thread_no - 1], "gs value for thread %ld is 0x%x\n", thread_no, gs_value);

    LDT_TABLE_ENTRY ldtEntries[FILL_ENTRIES];
    unsigned int sizeInBytes = sizeof(LDT_TABLE_ENTRY) * FILL_ENTRIES;
    int res                  = syscall(SYS_modify_ldt, LDT_GET, ldtEntries, sizeInBytes);
    if (res != sizeInBytes)
    {
        printf("LDT_GET failed\n");
        return 0;
    }
    if (((gs_value >> 2) & 1) == 0)
    {
        sprintf(buf[thread_no - 1], "%sGS points to GDT\n", buf[thread_no - 1]);
        for (unsigned int i = 0; i < FILL_ENTRIES; i++)
        {
            sprintf(buf[thread_no - 1], "%sThread %d: LDT entry %d\n", buf[thread_no - 1], thread_no, i);
        }
    }
    else
    {
        unsigned int entry = gs_value >> 3;
        if (LDTEntryA(entry) == ldtEntries[entry]._val1)
        {
            sprintf(buf[thread_no - 1], "%sThread %d: original LDT entry %d\n", buf[thread_no - 1], thread_no, entry);
        }
        else
        {
            sprintf(buf[thread_no - 1], "%sThread %d: modified LDT entry %d\n", buf[thread_no - 1], thread_no, entry);
        }
    }
    return 0;
}

int main(int argc, char* argv[])
{
    pthread_t h[NTHREADS];
    UserDesc tr[FILL_ENTRIES];
    int res;

    memset(tr, 0, sizeof(UserDesc) * FILL_ENTRIES);

    unsigned long num = pthread_self();

    for (unsigned int i = 1; i < FILL_ENTRIES; i++)
    {
        tr[i].entry_number = i;
        tr[i].base_addr    = i;
        res                = syscall(SYS_modify_ldt, LDT_SET, &tr[i], sizeof(UserDesc));
        if (res < 0)
        {
            printf("main thread: LDT_GET for entry %d failed, code %d, %s\n", i, errno, strerror(errno));
        }
    }
    LDT_TABLE_ENTRY ldtEntries[FILL_ENTRIES];
    unsigned int sizeInBytes = sizeof(LDT_TABLE_ENTRY) * FILL_ENTRIES;
    res                      = syscall(SYS_modify_ldt, LDT_GET, ldtEntries, sizeInBytes);
    if (res != sizeInBytes)
    {
        printf("LDT_GET failed\n");
        return 0;
    }
    for (unsigned int i = 1; i < FILL_ENTRIES; i++)
    {
        printf("ldt entry %d 0x%x \n", i, ldtEntries[i]._val1);
    }

    printf("Original GS for the main thread is 0x%x\n", TLS_GET_GS_REG());

    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        pthread_create(&h[i], 0, thread_func, (void*)i);
    }
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        pthread_join(h[i], 0);
    }
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        printf("%s", buf[i]);
    }
}

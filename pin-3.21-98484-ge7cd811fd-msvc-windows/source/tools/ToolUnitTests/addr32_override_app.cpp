/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

extern "C" int Do32BitOverride(int* dst, int* src);

#define MEMSIZE 1024
int src[MEMSIZE];
main(int argc, char* argv[])
{
    int* ptrTo32BitMem = (int*)mmap(NULL, MEMSIZE, PROT_READ | PROT_WRITE, MAP_32BIT | MAP_PRIVATE | MAP_ANON, -1, 0);
    printf("memory allocated at %p\n", ptrTo32BitMem);
    fflush(stdout);
    if ((long long)(ptrTo32BitMem)&0xffffffff00000000L)
    {
        printf("***Error did not get 32bit memory\n");
        exit(1);
    }
    memset(ptrTo32BitMem, 0, MEMSIZE);
    for (int i = 0; i < MEMSIZE; i++)
    {
        src[i] = 0xdeadbee0 + i;
    }
    Do32BitOverride(ptrTo32BitMem, src);
    printf("%x %x %x %x %x\n%x %x %x %x %x\n", ptrTo32BitMem[0], ptrTo32BitMem[1], ptrTo32BitMem[2], ptrTo32BitMem[3],
           ptrTo32BitMem[4], ptrTo32BitMem[5], ptrTo32BitMem[6], ptrTo32BitMem[7], ptrTo32BitMem[8], ptrTo32BitMem[9]);
    if (ptrTo32BitMem[0] != src[0] || ptrTo32BitMem[1] != 0 || ptrTo32BitMem[2] != src[2] || ptrTo32BitMem[3] != 0 ||
        ptrTo32BitMem[4] != src[4] || ptrTo32BitMem[5] != src[5] || ptrTo32BitMem[6] != 0 || ptrTo32BitMem[7] != src[4] ||
        ptrTo32BitMem[8] != src[5] || ptrTo32BitMem[9] != 0)
    {
        printf("***Error Do32BitOverride did not get expected values into ptrTo32BitMem\n");
        exit(1);
    }
    return (0);
}

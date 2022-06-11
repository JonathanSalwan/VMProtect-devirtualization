/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>

int main()
{
    int result;

    // Allocate memory in the low 32 bit of the address space
    void* ptr_32bit = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_32BIT, -1, 0);
    if (NULL == ptr_32bit)
    {
        perror("mmap failed");
        return 1;
    }

    asm("movq %1, %%rax\n"
        "movl $0xbadc0de, (%%rax)\n"
        "movl (%%eax), %%eax\n"
        "movl %%eax, %0\n"
        : "=m"(result)
        : "m"(ptr_32bit)
        : "%rax");
    printf("result = %x\n", result);
    return 0;
}

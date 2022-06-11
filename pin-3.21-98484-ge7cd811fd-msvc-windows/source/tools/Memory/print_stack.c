/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_TO_CONSUME 0x500

/*
 * This function will consume (at least) numPagesDoubled of half pages
 * (2KB each) from the stack memory.
 */
void consumeHalfPages(int numPagesDoubled)
{
    int i;
    char buf[2048];
    if (--numPagesDoubled > 0) consumeHalfPages(numPagesDoubled);
    for (i = 0; i < sizeof(buf); i++)
        buf[i] = (char)i;

    // Dummy print to prevent opt-out of 'buf'
    printf("%s", buf);
}

int main(int argc, char** argv, char** envp)
{
    intptr_t sp        = (intptr_t)&argc;
    size_t pgsize      = getpagesize();
    void* end_of_stack = NULL;
    sp &= ~(pgsize - 1);

    // Find the end of the stack (the page before the one that mprotect fails on)
    for (end_of_stack = (void*)sp; 0 == mprotect(end_of_stack + pgsize, pgsize, PROT_READ | PROT_WRITE); end_of_stack += pgsize)
        ;

    // print part of the stack in PIN's -reserve_memory file format
    printf("%p %p data 0\n", (void*)(sp - PAGE_TO_CONSUME * pgsize), end_of_stack);

    // The call below will consume at least PAGE_TO_CONSUME*4KB memory from the stack
    consumeHalfPages(PAGE_TO_CONSUME * 2);

    return 0;
}

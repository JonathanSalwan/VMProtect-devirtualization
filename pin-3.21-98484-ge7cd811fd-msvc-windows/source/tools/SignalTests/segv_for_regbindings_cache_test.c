/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "num_segvs.h"

#include <sys/mman.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

void handle(int, siginfo_t*, void*);
void make_segv();

int main()
{
    int i;
    struct sigaction sigact;

    sigact.sa_sigaction = handle;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable handle signal\n");
        return 1;
    }

    for (i = 0; i < NUM_SEGVS; i++)
    {
        make_segv();
    }
    return 0;
}

void make_segv()
{
    volatile int* p;
    int i;
    size_t pagesize = getpagesize();
    void* ptr       = mmap(0, 2 * pagesize, (PROT_READ | PROT_WRITE), (MAP_PRIVATE | MAP_ANONYMOUS), -1, 0);
    char* unmap     = ((char*)ptr) + pagesize + 0x20;
    munmap((char*)ptr + pagesize, pagesize);

    p = (volatile int*)unmap;
    i = *p;
}

int numSegvsHandled = 0;
void handle(int sig, siginfo_t* info, void* vctxt)
{
    numSegvsHandled++;
    printf("Got signal %d #sginals %d\n", sig, numSegvsHandled);
    if (numSegvsHandled >= NUM_SEGVS)
    {
        exit(0);
    }
}

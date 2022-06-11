/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void handle(int, siginfo_t*, void*);
void make_segv();
size_t getpagesize();

int main()
{
    struct sigaction sigact;

    sigact.sa_sigaction = handle;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable handle signal\n");
        return 1;
    }

    make_segv();
    return 0;
}

void handle(int sig, siginfo_t* info, void* vctxt)
{
    printf("Got signal %d\n", sig);
    exit(0);
}

#if defined(TARGET_MAC)
// Make segv by taking an address on the stack and go page by page
// until reaching an unmapped page.
void make_segv()
{
    size_t pagesize = getpagesize();
    int i           = 0;
    volatile int* p = &i;

    while (1)
    {
        p += pagesize;
        i += *p;
    }
}
#else
void make_segv()
{
    volatile int* p;
    int i;

    p = (volatile int*)0x9;
    i = *p;
}
#endif

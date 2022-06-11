/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sys/personality.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifndef ADDR_NO_RANDOMIZE
#define ADDR_NO_RANDOMIZE 0x40000
#endif

void disableASLR()
{
    int persona = personality(0xffffffff);
    if (!(persona & ADDR_NO_RANDOMIZE))
    {
        // We are running with ASLR: disable it and re-run the program
        personality(persona | ADDR_NO_RANDOMIZE);
    }
#ifdef TARGET_IA32
    // On 32 bit Linux, unlimiting the stack size is also required so the kernel won't
    // randomize the VDSO address
    struct rlimit rlim = {RLIM_INFINITY, RLIM_INFINITY};
    setrlimit(RLIMIT_STACK, &rlim);
#endif
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Use: %s <command to execute>\n", argv[0]);
        return 1;
    }

    disableASLR();

    execv(argv[1], argv + 1);
    fprintf(stderr, "ERROR: Could not to exec %s\n", argv[1]);
    return -1;
}

/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

#define CHECKNULL(P)                                       \
    if ((P) != 0)                                          \
    {                                                      \
        fprintf(stderr, "Failed: expected 0 got %p\n", P); \
        exit(1);                                           \
    }

KNOB< INT32 > KnobMaxSize(KNOB_MODE_WRITEONCE, "pintool", "m", "0x6000000", "Total bytes to allocate");
KNOB< INT32 > KnobIncrement(KNOB_MODE_WRITEONCE, "pintool", "i", "100", "Bytes to malloc each time");

VOID MalMalloc()
{
    for (INT32 size = 0; size < KnobMaxSize; size += KnobIncrement)
    {
        VOID* m = malloc(KnobIncrement);
        if (m == 0)
        {
            fprintf(stderr, "Failed malloc\n");
            PIN_ExitApplication(1);
        }
    }
}

int main(INT32 argc, CHAR** argv)
{
// Supress this warning for GCC7 since it doesn't allow passing 0xff000000 to malloc
#if defined(TARGET_IA32)
#if defined(__GNUC__)
#if (__GNUC__ >= 7)
#pragma GCC diagnostic ignored "-Walloc-size-larger-than="
#endif
#endif

    // Test the initial pool
    void* p = malloc(0xff000000);
    CHECKNULL(p);
#endif

    PIN_Init(argc, argv);

    MalMalloc();

#if defined(TARGET_IA32)
    // Test after we are out of initial pool
    void* p2 = malloc(0xff000000);
    CHECKNULL(p2);
#endif

    // Never returns
    PIN_StartProgram();

    return 0;
}

/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#ifdef TARGET_WINDOWS
#define NOINLINE __declspec(noinline)
#define IMPORT EXTERNC __declspec(dllimport)
#define EXPORT_CSYM EXTERNC __declspec(dllexport)
#else
#define NOINLINE __attribute__((noinline))
#define IMPORT EXTERNC
#define EXPORT_CSYM EXTERNC
#endif

extern int IMPORT one();

int EXPORT_CSYM NOINLINE two() { return 2; }

int (*two_ptr)(void);

int EXPORT_CSYM main(void)
{
    two_ptr = two;
    printf("%d\n", one());
    printf("%d\n", (*two_ptr)());
    return 0;
}

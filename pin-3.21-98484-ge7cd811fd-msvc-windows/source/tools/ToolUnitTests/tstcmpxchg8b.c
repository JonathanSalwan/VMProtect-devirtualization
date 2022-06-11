/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#if defined(__GNUC__)
#include <stdlib.h>
#endif

#ifdef TARGET_WINDOWS
#define ASMNAME(name)
#else
#define ASMNAME(name) asm(name)
#endif

extern int cmpxchg8_base(int* buff) ASMNAME("cmpxchg8_base");
extern int cmpxchg8_plus8(int* buff) ASMNAME("cmpxchg8_plus8");
extern int cmpxchg8_esp(int* buff) ASMNAME("cmpxchg8_esp");

int main()
{
    int arr[4];
    int res;
    arr[0] = 1;
    arr[1] = 1;
    printf("calling cmpxchg with base array\n");
    res = cmpxchg8_base(arr);
    if (res != 1 || arr[0] != 2 || arr[1] != 2)
    {
        printf("cmpxchg function failed: expected (2,2) got (%d,%d)\n", arr[0], arr[1]);
        exit(1);
    }

    printf("cmpxchg function ok (%d,%d)\n", arr[0], arr[1]);

    arr[2] = 1;
    arr[3] = 1;
    printf("calling cmpxchg with base plus 8\n");
    res = cmpxchg8_plus8(arr);
    if (res != 1 || arr[2] != 2 || arr[3] != 2)
    {
        printf("cmpxchg function failed: expected (2,2) got (%d,%d)\n", arr[2], arr[3]);
        exit(1);
    }

    printf("cmpxchg function ok (%d,%d)\n", arr[2], arr[3]);

    arr[2] = 1;
    arr[3] = 1;
    printf("calling cmpxchg with base esp\n");
    res = cmpxchg8_esp(arr);
    if (res != 1)
    {
        printf("cmpxchg function failed: expected (2,2) got (%d,%d)\n", arr[2], arr[3]);
        exit(1);
    }

    printf("cmpxchg function ok\n");
    exit(0);
}
